/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLKernelLoader.h - Kernel Loader main header                    */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLKERNELLOADER__
#define __SYSTEMLOADER_KERNEL_SLKERNELLOADER__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMach-O.h>

#define kCXLowMemoryString "Corona System " kCXSystemName " " kCXSystemRevision "." kCXSystemMajorVersion ""

#if !kCXAssemblyCode && kCXBootloaderCode

typedef struct {
    struct __SLMemoryZone {
        OSAddress physical;
        OSAddress virtual;
        OSAddress end;
        OSSize length;

        bool isAvailable;
        bool isCorrupt;

        bool bootloader;
    } *zones;

    SLMemoryMap *bootloaderMap;
    OSCount zoneCount;

    OSSize availableSize;
    OSSize corruptSize;

    OSSize fullSize;
} SLMemoryZoneInfo;

typedef struct __SLMemoryZone SLMemoryZone;

OSExport void CXKernelLoaderMain(SLMachOFile *loadedImage);

OSPrivate void SLSetupMem0(void);

OSExport SLMemoryZoneInfo *gSLSystemZoneInfo;
OSExport SLMachOFile *gSLMachOFileSelf;

OSExport OSAddress gSLBootXAddress;
OSExport OSSize gSLBootXSize;

#endif /* !kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLKERNELLOADER__) */
