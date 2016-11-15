#include <SystemLoader/SystemLoader.h>

#if kCXDebug

typedef struct {
    SLConsole rootConsole;
    SLGraphicsContext *context;
    SLBitmapFont *selectedFont;
    SLGraphicsPoint cursor;
    UInt32 baseOffsetX;
    UInt32 baseOffsetY;
    UInt32 color;
    UInt32 backgroundColor;
} SLVideoConsole;

void SLVideoConsoleSanitizeCursor(SLVideoConsole *console)
{
    if (console->cursor.x >= console->rootConsole.width)
    {
        console->cursor.x = 0;
        console->cursor.y++;
    }

    if (console->cursor.y >= console->rootConsole.height)
    {
        UInt32 rows = (console->rootConsole.height - console->cursor.y) + 1;

        OSCount blankPixelCount = console->context->width * rows * console->selectedFont->height;
        OSAddress blankOffset = (console->context->framebuffer + console->context->pixelCount) - blankPixelCount;
        OSAddress framebufferOffset = console->context->framebuffer + blankPixelCount;
        OSSize pixelCount = console->context->pixelCount - blankPixelCount;

        CXKMemoryCopy(framebufferOffset, console->context->framebuffer, pixelCount * sizeof(UInt32));
        CXKMemorySetValue(blankOffset, blankPixelCount * sizeof(UInt32), 0x00);

        console->cursor.y -= rows;
    }
}

void SLVideoConsoleOutputCharacter(UInt8 character, SLVideoConsole *console)
{
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
                    console->cursor.x = console->rootConsole.width - 1;
                    console->cursor.y--;
                }
            } else {
                console->cursor.x--;
            }
        } break;
        default: {
            SLGraphicsContextWritePrerenderedCharacter(console->context, character, outputLocation, console->selectedFont);
            console->cursor.x++;
        } break;
    }

    SLVideoConsoleSanitizeCursor(console);
}

void SLVideoConsoleOutput(UInt8 *string, OSSize size, SLVideoConsole *console)
{
    for (UInt8 i = 0; i < size; i++)
        SLVideoConsoleOutputCharacter(string[i], console);
}

UInt8 SLVideoConsoleInput(bool wait, SLVideoConsole *console)
{
    return kOSUTF8Error;
}

void SLVideoConsoleMoveBackward(OSCount spaces, SLVideoConsole *console)
{
    UInt8 *backspaces = SLAllocate(spaces).address;
    CXKMemorySetValue(backspaces, spaces, '\b');
    SLVideoConsoleOutput(backspaces, spaces, console);
    SLFree(backspaces);
}

void SLVideoConsoleDeleteCharacters(OSCount count, SLVideoConsole *console)
{
    UInt8 *spaces = SLAllocate(count).address;
    CXKMemorySetValue(spaces, count, '\b');
    SLVideoConsoleOutput(spaces, count, console);
    CXKMemorySetValue(spaces, count, ' ');
    SLVideoConsoleOutput(spaces, count, console);
    CXKMemorySetValue(spaces, count, '\b');
    SLVideoConsoleOutput(spaces, count, console);
    SLFree(spaces);
}

void __SLVideoConsoleInitAll(void)
{
    SLConfigFile *config = SLConfigGet();

    if (!config->dev.videoConsole.enabled)
        return;

    SLGraphicsOutput **screens = SLGraphicsOutputGetAll();
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
        CXKMemorySetValue(context->framebuffer, context->framebufferSize, 0x00);
        SLVideoConsole *console = SLAllocate(sizeof(SLVideoConsole)).address;

        console->selectedFont = &gSLBitmapFont8x16;
        console->context = context;
        console->cursor = ((SLGraphicsPoint){0, 0});

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
        console->rootConsole.height = context->height / console->selectedFont->height;
        console->baseOffsetY =       (context->height % console->selectedFont->height) / 2;
        console->rootConsole.width  = context->width  / console->selectedFont->width;
        console->baseOffsetX =       (context->width  % console->selectedFont->width)  / 2;
        console->rootConsole.deleteCharacters = SLVideoConsoleDeleteCharacters;
        console->rootConsole.moveBackward = SLVideoConsoleMoveBackward;

        SLRegisterConsole(&console->rootConsole);
    }
}

#endif /* kCXDebug */
