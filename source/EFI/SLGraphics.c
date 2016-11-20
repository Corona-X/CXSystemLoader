#include <SystemLoader/EFI/SLGraphics.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

SLGraphicsOutput **SLGraphicsOutputGetAll(void)
{
    SLBootServicesCheck(kOSNullPointer);

    SLBootServices *bootServices = SLBootServicesGetCurrent();
    SLProtocol protocol = kSLGraphicsOutputProtocol;
    OSAddress *devices;
    UIntN count;

    SLStatus status = bootServices->localeHandles(kSLSearchTypeByProtocol, &protocol, kOSNullPointer, &count, &devices);
    if (SLStatusIsError(status)) return kOSNullPointer;

    SLGraphicsOutput **results = SLAllocate((count + 1) * sizeof(SLGraphicsOutput *)).address;
    results[count] = kOSNullPointer;

    for (OSCount i = 0; i < count; i++)
    {
        SLGraphicsOutput *output;
        status = bootServices->handleProtocol(devices[i], &protocol, &output);
        results[i] = output;

        if (SLStatusIsError(status)) goto failure;
    }

    if (!SLBootServicesFree(devices)) goto failure;
    return results;

failure:
    SLBootServicesFree(devices);
    SLFree(results);
    
    return kOSNullPointer;
}

SLGraphicsModeInfo *SLGraphicsOutputGetMode(SLGraphicsOutput *graphics, UInt32 modeNumber)
{
    SLBootServicesCheck(kOSNullPointer);

    OSSize size = sizeof(SLGraphicsModeInfo *);
    SLGraphicsModeInfo *info;

    SLStatus status = graphics->getMode(graphics, modeNumber, &size, &info);
    bool failure = SLStatusIsError(status);

    return (failure ? kOSNullPointer : info);
}

SLGraphicsMode *SLGraphicsOutputGetCurrentMode(SLGraphicsOutput *graphics)
{
    SLBootServicesCheck(kOSNullPointer);

    return graphics->mode;
}

SLGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics)
{
    return SLGraphicsOutputGetContextWithMaxSize(graphics, ~((UInt32)0), ~((UInt32)0));
}

SLGraphicsContext *SLGraphicsOutputGetContextWithMaxSize(SLGraphicsOutput *graphics, UInt32 maxHeight, UInt32 maxWidth)
{
    SLBootServicesCheck(kOSNullPointer);

    UInt32 modes = SLGraphicsOutputGetCurrentMode(graphics)->numberOfModes;
    SLGraphicsModeInfo *maxMode = kOSNullPointer;
    UInt32 maxModeNumber = 0;
    UInt32 maxModeHeight = 0;
    UInt32 maxModeWidth = 0;

    for (UInt32 i = 0; i < modes; i++)
    {
        SLGraphicsModeInfo *mode = SLGraphicsOutputGetMode(graphics, i);

        if (mode->format != kSLGraphicsPixelFormatRGBX8 && mode->format != kSLGraphicsPixelFormatBGRX8)
            continue;

        if (mode->width > maxWidth)
            continue;

        if (mode->width > maxModeWidth)
            maxModeWidth = mode->width;
    }

    for (UInt32 i = 0; i < modes; i++)
    {
        SLGraphicsModeInfo *mode = SLGraphicsOutputGetMode(graphics, i);

        if (mode->format != kSLGraphicsPixelFormatRGBX8 && mode->format != kSLGraphicsPixelFormatBGRX8)
            continue;

        if (mode->width == maxModeWidth)
        {
            if (mode->height > maxHeight)
                continue;

            if (mode->height > maxModeHeight)
            {
                maxModeHeight = mode->height;
                maxModeNumber = i;
                maxMode = mode;
            }
        }
    }

    if (!maxMode) return kOSNullPointer;
    SLStatus status = graphics->setMode(graphics, maxModeNumber);
    if (SLStatusIsError(status)) return kOSNullPointer;
    SLGraphicsMode *mode = SLGraphicsOutputGetCurrentMode(graphics);

    SLGraphicsContext *context = SLAllocate(sizeof(SLGraphicsContext)).address;
    context->height = mode->info->height;
    context->width = mode->info->width;
    context->framebuffer = mode->framebuffer;
    context->framebufferSize = mode->framebufferSize;
    context->pixelCount = mode->framebufferSize / sizeof(UInt32);
    context->isBGRX = (mode->info->format == kSLGraphicsPixelFormatBGRX8);

    return context;
}

void SLGraphicsContextWriteCharacter(SLGraphicsContext *context, UInt8 character, SLGraphicsPoint location, SLBitmapFont *font, UInt32 color, UInt32 bgColor)
{
    UInt32 *rowPointer = context->framebuffer + ((location.y * context->width) + location.x);
    UInt8 *characterData = font->packedData + (character * font->height);
    
    for (OSCount y = 0; y < font->height; y++)
    {
        UInt8 data = characterData[y];

        for (SInt8 x = (font->width - 1); x >= 0; x--)
        {
            UInt8 state = (data >> x) & 1;
            UInt32 fillValue = (state ? color : bgColor);

            rowPointer[x] = fillValue;
        }

        rowPointer += context->width;
    }
}

void SLGraphicsContextWritePrerenderedCharacter(SLGraphicsContext *context, UInt8 character, SLGraphicsPoint location, SLBitmapFont *font, UInt32 color, UInt32 backgroundColor)
{
    UInt32 *rowPointer = context->framebuffer + ((location.y * context->width) + location.x);
    UInt8 *characterData = font->fontData + ((font->height * font->width) * character);
    OSCount y = font->height - 1;

    do {
        for (UInt8 i = 0; i < font->width; i++)
            rowPointer[i] = (characterData[i] ? color : backgroundColor);

        characterData += font->width;
        rowPointer += context->width;
    } while (y--);
}

#if kCXBuildDev
    void SLGraphicsOutputDumpInfo(void)
    {
        SLBootServicesCheck((void)(0));
    }
#endif /* kCXBuildDev */
