/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLVideo.h - Kernel Loader screen interface                      */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 21.11.2018 -  2:30 PM TP                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLVIDEO__
#define __SYSTEMLOADER_KERNEL_SLVIDEO__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLGraphics.h>

#define kXKBootConfigKeyScreenHeightMax  "Screen-Height-Max"
#define kXKBootConfigKeyScreenWidthMax   "Screen-Width-Max"

#if !kCXAssemblyCode && kCXBootloaderCode

typedef struct {
    UInt32 height;
    UInt32 width;
    UInt32 *framebuffer;
    OSSize framebufferSize;
    OSCount pixelCount;
    bool isBGRX;
} SLGraphicsContext;

OSPrivate SLGraphicsContext *SLGraphicsOutputGetContextWithMaxSize(SLGraphicsOutput *graphics, UInt32 maxHeight, UInt32 maxWidth);
OSPrivate SLGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics);

OSPrivate void SLDrawLogoInContext(SLGraphicsContext *context);
OSPrivate void SLSetupVideo(void);

#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLVIDEO__) */
