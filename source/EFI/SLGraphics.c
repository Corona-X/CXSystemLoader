#include <SystemLoader/SystemLoader.h>

SLGraphicsOutput **SLGraphicsOutputGetAll(void)
{
    SLBootServices *bootServices = gSLLoaderSystemTable->bootServices;
    SLProtocol protocol = kSLGraphicsOutputProtocol;
    OSAddress *devices;
    UIntN count;
    
    SLStatus status = bootServices->localeHandles(kSLSearchTypeByProtocol, &protocol, kOSNullPointer, &count, &devices);
    if (SLStatusIsError(status)) return kOSNullPointer;
    
    SLGraphicsOutput **results = SLAllocate((count + 1) * sizeof(SLGraphicsOutput *)).address;
    results[count] = kOSNullPointer;
    OSCount i = 0;
    
    for ( ; i < count; i++)
    {
        SLGraphicsOutput *output;
        status = bootServices->handleProtocol(devices[i], &protocol, &output);
        results[i] = output;
        
        if (SLStatusIsError(status)) goto failure;
    }
    
    status = bootServices->free(devices);
    if (SLStatusIsError(status)) goto failure;
    return results;
    
failure:
    bootServices->free(devices);
    SLFree(results);
    
    return kOSNullPointer;
}

SLGraphicsModeInfo *SLGraphicsOutputGetMode(SLGraphicsOutput *graphics, UInt32 modeNumber)
{
    OSSize size = sizeof(SLGraphicsModeInfo *);
    SLGraphicsModeInfo *info;
    
    SLStatus status = graphics->getMode(graphics, modeNumber, &size, &info);
    return (SLStatusIsError(status) ? kOSNullPointer : info);
}

SLGraphicsMode *SLGraphicsOutputGetCurrentMode(SLGraphicsOutput *graphics)
{
    return graphics->mode;
}

SLGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics)
{
    return SLGraphicsOutputGetContextWithMaxSize(graphics, 0xFFFFFFFF, 0xFFFFFFFF);
}

SLGraphicsContext *SLGraphicsOutputGetContextWithMaxSize(SLGraphicsOutput *graphics, UInt32 maxHeight, UInt32 maxWidth)
{
    UInt32 modes = graphics->mode->numberOfModes;
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

    SLGraphicsContext *context = SLAllocate(sizeof(SLGraphicsContext)).address;
    context->height = maxMode->height;
    context->width = maxMode->width;
    context->framebuffer = graphics->mode->framebuffer;
    context->framebufferSize = graphics->mode->framebufferSize;
    context->pixelCount = graphics->mode->framebufferSize / sizeof(UInt32);
    context->isBGRX = (maxMode->format == kSLGraphicsPixelFormatBGRX8);

    return context;
}

void SLGraphicsContextWriteCharacter(SLGraphicsContext *context, UInt8 character, SLGraphicsPoint location, SLBitmapFont *font, UInt32 color, UInt32 bgColor)
{
    // This only supports the 8x16 bitmap font for now...
    if (font->height != 16 || font->width != 8) return;
    
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

void SLGraphicsContextWritePrerenderedCharacter(SLGraphicsContext *context, UInt8 character, SLGraphicsPoint location, SLBitmapFont *font)
{
    UInt32 *rowPointer = context->framebuffer + ((location.y * context->width) + location.x);
    UInt32 *characterData = font->fontData + ((font->height * font->width) * character);
    OSCount y = font->height - 1;
    
    do {
        CXKMemoryCopy(characterData, rowPointer, font->width * sizeof(UInt32));
        
        characterData += font->width;
        rowPointer += context->width;
    } while (y--);
}

#if kCXBuildDev
    void SLGraphicsOutputDumpInfo(void)
    {
        //
    }
#endif /* kCXBuildDev */
