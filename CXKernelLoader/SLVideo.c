#include <SystemLoader/Kernel/SLVideo.h>

#include <Kernel/Shared/XKDebugLog.h>
#include <Kernel/C/XKMemory.h>

#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

OSPrivate XKGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics);
OSPrivate XKGraphicsContext *SLGraphicsOutputGetContextWithMaxSize(SLGraphicsOutput *graphics, UInt32 maxHeight, UInt32 maxWidth);

XKGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics)
{
    return SLGraphicsOutputGetContextWithMaxSize(graphics, ~((UInt32)0), ~((UInt32)0));
}

XKGraphicsContext *SLGraphicsOutputGetContextWithMaxSize(SLGraphicsOutput *graphics, UInt32 maxHeight, UInt32 maxWidth)
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

    XKGraphicsContext *context = SLAllocate(sizeof(XKGraphicsContext));
    context->height = mode->info->height;
    context->width = mode->info->width;
    context->framebuffer = mode->framebuffer;
    context->framebufferSize = mode->framebufferSize;
    context->pixelCount = mode->framebufferSize / sizeof(UInt32);
    context->isBGRX = (mode->info->format == kSLGraphicsPixelFormatBGRX8);

    return context;
}

#include "XKBootLogo.h"

OSPrivate void SLDrawLogoInContext(XKGraphicsContext *context);
OSPrivate void SLSetupVideo(void);

void SLDrawLogoInContext(XKGraphicsContext *context)
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
    SLPrintString("We have screens at %p\n", screens);

    if (!screens)
    {
        SLPrintString("No output screens found!\n");
        return;
    }

    SLGraphicsOutput *screen = *screens;

    XKGraphicsContext *context = SLGraphicsOutputGetContextWithMaxSize(screen, 1400, 1050);
    if (!context) goto end;

    XKMemorySetValue(context->framebuffer, context->framebufferSize, 0xFF);
    SLDrawLogoInContext(context);

    SLFree(context);

end:
    SLFree(screens);
}
