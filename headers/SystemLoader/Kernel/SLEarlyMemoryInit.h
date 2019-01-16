/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLEarlyMemoryInit.h - Kernel loader early memory initialization */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  1.15.2019 -  6:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLEARLYMEMORYINIT__
#define __SYSTEMLOADER_KERNEL_SLEARLYMEMORYINIT__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLBootServices.h>

#if !kCXAssemblyCode && kCXBootloaderCode

typedef struct {
    struct __SLMemoryZone {
        OSAddress physical;
        OSAddress virtual;
        OSAddress end;
        OSSize length;

        bool isAvailable;
        bool isCorrupt;
    } *zones;

    SLMemoryMap *bootloaderMap;
    OSCount zoneCount;

    OSSize availableSize;
    OSSize corruptSize;

    OSSize fullSize;
} SLMemoryZoneInfo;

typedef struct __SLMemoryZone SLMemoryZone;

OSPrivate SLMemoryZoneInfo *SLReadMemoryMap(SLMemoryMap *map);
OSPrivate SLMemoryZoneInfo *SLMemoryZoneRead(SLMemoryMap *map);

OSPrivate void SLDumpMemoryInfo(SLMemoryZoneInfo *info);
OSPrivate void SLDumpMemoryMap(SLMemoryMap *map);

#endif /* !kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLEARLYMEMORYINIT__) */
