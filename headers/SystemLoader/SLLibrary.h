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
    OSPrivate void SLDumpConsoles(void);

    OSExport OSAddress gSLFirmwareReturnAddress;
    OSExport SLSystemTable *gSLLoaderSystemTable;
    OSExport OSAddress gSLLoaderImageHandle;
    OSExport bool gSLBootServicesEnabled;

    #if kCXBuildDev
        OSPrivate bool SLPromptUser(const char *s);
        OSPrivate void SLShowDelay(const char *s, UInt64 seconds);

        OSPrivate void SLDumpProcessorState(bool standard, bool system, bool debug);
        OSPrivate void SLPrintBasicState(XKProcessorBasicState *state);
        OSPrivate void SLPrintSystemState(XKProcessorSystemState *state);
        OSPrivate void SLPrintDebugState(XKProcessorDebugState *state);

        OSPrivate void __XKInputConsoleInitAllEFI(void);
        OSPrivate void __XKSerialConsoleInitAll(void);
        OSPrivate void __XKVideoConsoleInitAll(void);
        OSPrivate void __XKBitmapFontInitialize(void);
        OSPrivate void __SLLibraryInitialize(void);

        OSExport OSSize gSLLoaderImageSize;
    #else /* !kCXBuildDev */
        #define SLPrintString(s, ...) do {} while(0)
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLLIBRARY__) */
