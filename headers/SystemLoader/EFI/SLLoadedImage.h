/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLoadedImage.h - Loaded EFI Image Functions                    */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLLOADEDIMAGE__
#define __SYSTEMLOADER_SLLOADEDIMAGE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLFile.h>

#if !kCXAssemblyCode

typedef struct {
    UInt32 revision;
    OSAddress unused1[2];
    OSAddress deviceHandle;
    //SLDevicePath *filePath;
    OSAddress filePath;
    OSAddress reserved;
    OSAddress unused2;
    UInt64 unused3;
    UInt32 unused4[2];
    OSAddress unused5;
} SLLoadedImage;

#if kCXBootloaderCode
    OSPrivate SLLoadedImage *SLLoadedImageGetFromHandle(OSAddress imageHandle);
    OSPrivate SLFile *SLLoadedImageGetRoot(SLLoadedImage *image);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLLOADEDIMAGE__) */
