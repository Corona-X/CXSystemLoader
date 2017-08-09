/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLoadedImage.h - Loaded EFI Image Functions                    */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLLOADEDIMAGE__
#define __SYSTEMLOADER_EFI_SLLOADEDIMAGE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/EFI/SLFile.h>

#if !kCXAssemblyCode

typedef struct {
    UInt32          revision;
    OSAddress       parentHandle;
    SLSystemTable   *systemTable;
    OSAddress       deviceHandle;
    OSAddress       filePath;
    OSAddress       reserved;
    UInt32          loadOptionsSize;
    OSAddress       loadOptions;
    OSAddress       imageBase;
    UInt64          imageSize;
    SLMemoryType    codeType;
    SLMemoryType    dataType;
    OSAddress       unload;
} SLLoadedImage;

#if kCXBootloaderCode
    OSPrivate SLLoadedImage *SLLoadedImageGetFromHandle(OSAddress imageHandle);
    OSPrivate SLFile *SLLoadedImageGetRoot(SLLoadedImage *image);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLLOADEDIMAGE__) */
