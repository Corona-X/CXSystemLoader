/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLCorePower.h - Initialization Methods for CorePower (ACPI)     */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  1.11.2019 - 10:00 AM TPE                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLCOREPOWER__
#define __SYSTEMLOADER_KERNEL_SLCOREPOWER__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/Kernel/SLKernelLoader.h>
#include <Kernel/CoreDriver/ACPI/CPBase.h>

#if !kCXAssemblyCode && kCXBootloaderCode

OSPrivate CPRootTable *SLCorePowerEarlyInit(OSAddress rootPointer);

#if kCXBuildDev
    OSPrivate void SLCorePowerDumpRootTables(CPRootTable *table);
#endif /* kCXBuildDev */

#endif /* !kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLCOREPOWER__) */
