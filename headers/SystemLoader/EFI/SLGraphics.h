/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLGraphics.h - EFI Graphics Output Structure and Functions      */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 30.10.2016 - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLGRAPHICS__
#define __SYSTEMLOADER_SLGRAPHICS__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <Kernel/XKGraphics.h>

#if !kCXAssemblyCode

typedef enum {
    kSLGraphicsPixelFormatRGBX8,
    kSLGraphicsPixelFormatBGRX8,
    kSLGraphicsPixelFormatBitMask,
    kSLGraphicsPixelFormatBLT
} SLGraphicsPixelFormat;

typedef struct {
    UInt32 red;
    UInt32 green;
    UInt32 blue;
    UInt32 reserved;
} SLGraphicsPixelMask;

typedef struct {
    UInt32 version;
    UInt32 width;
    UInt32 height;
    SLGraphicsPixelFormat format;
    SLGraphicsPixelMask pixelInfo;
    UInt32 pixelsPerScanline;
} SLGraphicsModeInfo;

typedef struct {
    UInt32 numberOfModes;
    UInt32 currentMode;
    SLGraphicsModeInfo *info;
    UIntN infoSize;
    OSAddress framebuffer;
    UIntN framebufferSize;
} SLGraphicsMode;

typedef struct {
    SLABI SLStatus (*getMode)(OSAddress this, UInt32 number, UIntN *infoSize, SLGraphicsModeInfo **info);
    SLABI SLStatus (*setMode)(OSAddress this, UInt32 number);
    OSAddress blockTransfer;
    SLGraphicsMode *mode;
} SLGraphicsOutput;

#if kCXBootloaderCode
    OSPrivate SLGraphicsOutput **SLGraphicsOutputGetAll(OSCount *count);
    OSPrivate SLGraphicsModeInfo *SLGraphicsOutputGetMode(SLGraphicsOutput *graphics, UInt32 modeNumber);
    OSPrivate SLGraphicsMode *SLGraphicsOutputGetCurrentMode(SLGraphicsOutput *graphics);

    OSPrivate XKGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics);
    OSPrivate XKGraphicsContext *SLGraphicsOutputGetContextWithMaxSize(SLGraphicsOutput *graphics, UInt32 maxHeight, UInt32 maxWidth);

    #if kCXBuildDev
        OSPrivate bool SLGraphicsOutputDumpInfo(void);
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLGRAPHICS__) */
