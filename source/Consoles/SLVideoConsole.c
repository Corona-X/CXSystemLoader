#include <SystemLoader/SLFormattedPrint.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLConfigFile.h>
#include <SystemLoader/EFI/SLGraphics.h>
#include <System/OSByteMacros.h>
#include <Kernel/XKMemory.h>

#if kCXDebug

typedef struct {
    SLConsole rootConsole;
    SLGraphicsContext *context;
    SLBitmapFont *selectedFont;
    SLGraphicsPoint cursor;
    UInt32 baseOffsetX;
    UInt32 baseOffsetY;
    UInt32 height;
    UInt32 width;
    UInt32 color;
    UInt32 backgroundColor;
    bool isBackground;
    bool inEscapeCode;
} SLVideoConsole;

void SLVideoConsoleSanitizeCursor(SLVideoConsole *console)
{
    if (console->cursor.x >= console->width)
    {
        console->cursor.x = 0;
        console->cursor.y++;
    }

    if (console->cursor.y >= console->height)
    {
        UInt32 rows = (console->height - console->cursor.y) + 1;

        OSCount blankPixelCount = console->context->width * rows * console->selectedFont->height;
        OSAddress blankOffset = (console->context->framebuffer + console->context->pixelCount) - blankPixelCount;
        OSAddress framebufferOffset = console->context->framebuffer + blankPixelCount;
        OSSize pixelCount = console->context->pixelCount - blankPixelCount;

        XKMemoryCopy(framebufferOffset, console->context->framebuffer, pixelCount * sizeof(UInt32));
        XKMemorySetValue(blankOffset, blankPixelCount * sizeof(UInt32), 0x00);

        console->cursor.y -= rows;
    }
}

void SLVideoConsoleOutputCharacter(UInt8 character, SLVideoConsole *console)
{
    if (console->inEscapeCode)
    {
        if (character == '\e')
        {
            console->inEscapeCode = false;
            return;
        }

        if (character == 'f')
        {
            console->isBackground = false;
            console->color = 0;
            return;
        }

        if (character == 'b')
        {
            console->isBackground = true;
            console->backgroundColor = 0;
            return;
        }

        char offset = 0;

        if (OSIsBetween('0', character, '9'))
            offset = '0';

        if (OSIsBetween('A', character, 'F'))
            offset = 'A' - 10;

        if (!offset) {
            if (!console->isBackground) {
                console->color = SLConfigGet()->dev.videoConsole.foregroundColor;
            } else {
                console->backgroundColor = SLConfigGet()->dev.videoConsole.backgroundColor;
            }

            console->inEscapeCode = false;
            SLPrintString("Error: Invalid escape sequence! (Extraneous '%c' [0x%02X])\n", character, character);
        } else {
            if (!console->isBackground) {
                console->color *= 0x10;
                console->color += (character - offset);
            } else {
                console->backgroundColor *= 0x10;
                console->backgroundColor += (character - offset);
            }
        }

        return;
    }

    SLGraphicsPoint outputLocation;
    outputLocation.y = (console->cursor.y * console->selectedFont->height) + console->baseOffsetY;
    outputLocation.x = (console->cursor.x * console->selectedFont->width ) + console->baseOffsetX;

    // This is just fun sometimes...
    //SLDelayProcessor(25000, !SLBootServicesHaveTerminated());

    switch (character)
    {
        case '\n': {
            console->cursor.x = 0;
            console->cursor.y++;
        } break;
        case '\r': {
            console->cursor.x = 0;
        } break;
        case '\b': {
            if (!console->cursor.x) {
                if (console->cursor.y)
                {
                    console->cursor.x = console->width - 1;
                    console->cursor.y--;
                }
            } else {
                console->cursor.x--;
            }
        } break;
        case '\t': {
            for (UInt8 i = 0; i < 4; i++)
                SLVideoConsoleOutputCharacter(' ', console);
        } return;
        case '\e': {
            console->inEscapeCode = true;
        } return;
        default: {
            SLGraphicsContextWriteCharacter(console->context, character, outputLocation, console->selectedFont, console->color, console->backgroundColor);
            //SLGraphicsContextWritePrerenderedCharacter(console->context, character, outputLocation, console->selectedFont, console->color, console->backgroundColor);
            console->cursor.x++;
        } break;
    }

    SLVideoConsoleSanitizeCursor(console);
}

void SLVideoConsoleOutput(OSUTF8Char *string, OSSize size, SLVideoConsole *console)
{
    for (UInt8 i = 0; i < size; i++)
        SLVideoConsoleOutputCharacter(string[i], console);
}

UInt8 SLVideoConsoleInput(bool wait, SLVideoConsole *console)
{
    return 0;
}

void SLVideoConsoleMoveBackward(OSCount spaces, SLVideoConsole *console)
{
    UInt8 *backspaces = SLAllocate(spaces).address;
    XKMemorySetValue(backspaces, spaces, '\b');
    SLVideoConsoleOutput(backspaces, spaces, console);
    SLFree(backspaces);
}

void SLVideoConsoleDeleteCharacters(OSCount count, SLVideoConsole *console)
{
    UInt8 *spaces = SLAllocate(count).address;
    XKMemorySetValue(spaces, count, '\b');
    SLVideoConsoleOutput(spaces, count, console);
    XKMemorySetValue(spaces, count, ' ');
    SLVideoConsoleOutput(spaces, count, console);
    XKMemorySetValue(spaces, count, '\b');
    SLVideoConsoleOutput(spaces, count, console);
    SLFree(spaces);
}

bool SLConsoleIsVideoConsole(SLConsole *console)
{
    return (console->output == (SLConsoleOutput)SLVideoConsoleOutput);
}

void __SLVideoConsoleInitAll(void)
{
    SLConfigFile *config = SLConfigGet();

    if (!config->dev.videoConsole.enabled)
        return;

    SLGraphicsOutput **screens = SLGraphicsOutputGetAll(kOSNullPointer);
    UInt8 count = 0;

    if (!screens)
    {
        SLPrintString("Aww, no screens :'(\n");
        return;
    }

    while (screens)
    {
        SLGraphicsOutput *screen = (*screens++);
        count++;

        if (count > config->dev.videoConsole.maxScreenCount)
            break;

        SLGraphicsContext *context = SLGraphicsOutputGetContextWithMaxSize(screen, config->dev.videoConsole.maxScreenHeight, config->dev.videoConsole.maxScreenWidth);
        XKMemorySetValue(context->framebuffer, context->framebufferSize, 0x00);
        SLVideoConsole *console = SLAllocate(sizeof(SLVideoConsole)).address;

        console->selectedFont = gSLBitmapFont8x16;
        console->context = context;
        console->cursor = ((SLGraphicsPoint){0, 0});
        console->inEscapeCode = false;
        console->height = context->height / console->selectedFont->height;
        console->width  = context->width  / console->selectedFont->width;

        if (context->isBGRX) {
            UInt32 backgroundColor = config->dev.videoConsole.backgroundColor;
            console->backgroundColor = ((backgroundColor & 0xFF0000) >> 16);
            console->backgroundColor |= (backgroundColor & 0x00FF00);
            console->backgroundColor |= (backgroundColor & 0x0000FF) << 16;

            UInt32 foregroundColor = config->dev.videoConsole.foregroundColor;
            console->color = ((foregroundColor & 0xFF0000) >> 16);
            console->color |= (foregroundColor & 0x00FF00);
            console->color |= (foregroundColor & 0x0000FF) << 16;
        } else {
            console->backgroundColor = config->dev.videoConsole.backgroundColor;
            console->color = config->dev.videoConsole.foregroundColor;
        }

        console->rootConsole.id = 0xFF;
        console->rootConsole.context = console;
        console->rootConsole.output = SLVideoConsoleOutput;
        console->rootConsole.input = SLVideoConsoleInput;
        console->baseOffsetY =       (context->height % console->selectedFont->height) / 2;
        console->baseOffsetX =       (context->width  % console->selectedFont->width)  / 2;
        console->rootConsole.deleteCharacters = SLVideoConsoleDeleteCharacters;
        console->rootConsole.moveBackward = SLVideoConsoleMoveBackward;

        SLRegisterConsole(&console->rootConsole);
    }
}

#endif /* kCXDebug */
