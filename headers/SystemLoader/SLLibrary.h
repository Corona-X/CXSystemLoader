/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLibrary.h - System Loader Library Function definitions        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 26.9.2016  - 8:45 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLLIBRARY__
#define __SYSTEMLOADER_SLLIBRARY__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLBasicIO.h>

#if !kCXAssemblyCode && kCXBootloaderCode

#define SLBootServicesCheck(e)                                                                          \
    do {                                                                                                \
        if (OSUnlikely(SLBootServicesHaveTerminated()))                                                 \
        {                                                                                               \
            SLPrintString("Function %s called after Boot Services Terminated!\n", __func__);            \
            return e;                                                                                   \
        }                                                                                               \
    } while (0)

OSPrivate SLABI OSNoReturn void SLLeave(SLStatus status);
OSPrivate OSNoReturn void SLUnrecoverableError(void);
OSPrivate bool SLDelayProcessor(UInt64 time);
OSPrivate OSAddress SLGetMainImageHandle(void);

OSExport OSAddress gSLFirmwareReturnAddress;
OSExport OSSize gSLLoaderImageSize;

// These are passed on to CXKernelLoader
OSExport OSAddress gSLLoaderImageHandle;
OSExport bool gSLBootServicesEnabled;

#endif /* kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLLIBRARY__) */
