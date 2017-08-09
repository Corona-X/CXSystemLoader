/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLGraphics.h - EFI Graphics Output Structure and Functions      */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 30.10.2016 - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLGRAPHICS__
#define __SYSTEMLOADER_EFI_SLGRAPHICS__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode

typedef OSEnum(UInt32) {
    kSLGraphicsPixelFormatRGBX8     = 0,
    kSLGraphicsPixelFormatBGRX8     = 1,
    kSLGraphicsPixelFormatBitMask   = 2,
    kSLGraphicsPixelFormatBLT       = 3
} SLGraphicsPixelFormat;

typedef struct {
    UInt32 red;
    UInt32 green;
    UInt32 blue;
    UInt32 reserved;
} SLGraphicsPixelMask;

typedef struct {
    UInt32                  version;
    UInt32                  width;
    UInt32                  height;
    SLGraphicsPixelFormat   format;
    SLGraphicsPixelMask     pixelInfo;
    UInt32                  pixelsPerScanline;
} SLGraphicsModeInfo;

typedef struct {
    UInt32                  numberOfModes;
    UInt32                  currentMode;
    SLGraphicsModeInfo      *info;
    OSSize                  infoSize;
    OSAddress               framebuffer;
    OSSize                  framebufferSize;
} SLGraphicsMode;

typedef struct {
    SLABI SLStatus (*getMode)(OSAddress this, UInt32 number, OSSize *infoSize, SLGraphicsModeInfo **info);
    SLABI SLStatus (*setMode)(OSAddress this, UInt32 number);
    OSAddress       blockTransfer;
    SLGraphicsMode *mode;
} SLGraphicsOutput;

#if kCXBootloaderCode
    OSPrivate SLGraphicsOutput **SLGraphicsOutputGetAll(OSCount *count);
    OSPrivate SLGraphicsModeInfo *SLGraphicsOutputGetMode(SLGraphicsOutput *graphics, UInt32 modeNumber);
    OSPrivate SLGraphicsMode *SLGraphicsOutputGetCurrentMode(SLGraphicsOutput *graphics);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLGRAPHICS__) */
