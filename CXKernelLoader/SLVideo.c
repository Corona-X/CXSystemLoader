#include <SystemLoader/Kernel/SLVideo.h>

#include <Kernel/Shared/XKBootConfig.h>
#include <Kernel/C/XKMemory.h>

#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

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

    SLGraphicsContext *context = SLAllocate(sizeof(SLGraphicsContext));
    context->height = mode->info->height;
    context->width = mode->info->width;
    context->framebuffer = mode->framebuffer;
    context->framebufferSize = mode->framebufferSize;
    context->pixelCount = mode->framebufferSize / sizeof(UInt32);
    context->isBGRX = (mode->info->format == kSLGraphicsPixelFormatBGRX8);

    return context;
}

SLGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics)
{
    return SLGraphicsOutputGetContextWithMaxSize(graphics, ~((UInt32)0), ~((UInt32)0));
}

#include "XKBootLogo.h"

void SLDrawLogoInContext(SLGraphicsContext *context)
{
    OSIndex y = (context->height - kXKBootLogoHeight) / 2;
    OSIndex x = (context->width - kXKBootLogoWidth) / 2;

    for (OSIndex i = 0; i < (OSIndex)(kXKBootLogoHeight); i++)
    {
        for (OSIndex j = 0; j < (OSIndex)(kXKBootLogoWidth); j++)
        {
            UInt64 frameBufferIndex = ((i + y) * context->width) + (j + x);
            UInt64 imageindex = (i * kXKBootLogoWidth) + j;

            UInt32 color = kXKBootLogoPaletteRGB[kXKBootLogoData[imageindex]];
            context->framebuffer[frameBufferIndex] = color;
        }
    }
}

void SLSetupVideo(void)
{
    SLGraphicsOutput **screens = SLGraphicsOutputGetAll(kOSNullPointer);

    if (!screens) {
        SLPrintString("No output screens found!\n");
        return;
    } else {
        SLPrintString("We have screens at %p\n", screens);
    }

    SLGraphicsOutput *screen = *screens;

    UInt32 height = (UInt32)XKBootConfigGetNumber(gXKBootConfig, kXKBootConfigKeyScreenHeightMax, 0);
    UInt32 width = (UInt32)XKBootConfigGetNumber(gXKBootConfig, kXKBootConfigKeyScreenWidthMax, 0);

    if (!height)
        height = ~height;

    if (!width)
        width = ~width;

    SLGraphicsContext *context = SLGraphicsOutputGetContextWithMaxSize(screen, width, height);
    if (!context) goto end;

    XKMemorySetValue(context->framebuffer, context->framebufferSize, 0xFF);
    SLDrawLogoInContext(context);

    SLFree(context);

end:
    SLFree(screens);
}
