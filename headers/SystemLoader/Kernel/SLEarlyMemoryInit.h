/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLEarlyMemoryInit.h - Kernel loader early memory initialization */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  1.15.2019 -  6:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLEARLYMEMORYINIT__
#define __SYSTEMLOADER_KERNEL_SLEARLYMEMORYINIT__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/Kernel/SLKernelLoader.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMach-O.h>

#if !kCXAssemblyCode && kCXBootloaderCode

OSPrivate SLMemoryZoneInfo *SLProcessMemoryMap(SLMemoryMap *map);
OSPrivate bool SLDoEarlyMemoryInit(SLMemoryMap *map);

#if kCXBuildDev
    OSPrivate void SLDumpMemoryZoneInfo(SLMemoryZoneInfo *info);
    OSPrivate void SLDumpMemoryMap(SLMemoryMap *map);
#endif /* kCXBuildDev */

#endif /* !kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLEARLYMEMORYINIT__) */
