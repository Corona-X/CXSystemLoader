/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLibrary.h - System Loader Library Function definitions        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 26.9.2016  - 8:45 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLLIBRARY__
#define __SYSTEMLOADER_SLLIBRARY__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <Kernel/XKProcessorState.h>

#if !kCXAssemblyCode

#if kCXBootloaderCode
    #define SLBootServicesCheck(e)                                                                          \
        do {                                                                                                \
            if (SLBootServicesHaveTerminated())                                                             \
            {                                                                                               \
                XKLog(kXKLogLevelError, "Function %s called after Boot Services Terminated!\n", __func__);  \
                return (e);                                                                                 \
            }                                                                                               \
        } while (0)

    OSPrivate SLABI OSNoReturn void SLLeave(SLStatus status);
    OSPrivate OSAddress SLGetMainImageHandle(void);
    OSPrivate OSNoReturn void SLUnrecoverableError(void);
    OSPrivate bool SLDelayProcessor(UIntN time, bool useBootServices);

    OSExport OSAddress gSLFirmwareReturnAddress;
    OSExport SLSystemTable *gSLLoaderSystemTable;
    OSExport OSAddress gSLLoaderImageHandle;
    OSExport bool gSLBootServicesEnabled;
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLLIBRARY__) */
