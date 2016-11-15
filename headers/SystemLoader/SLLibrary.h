/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLibrary.h - System Loader Library Function definitions        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 26.9.2016  - 8:45 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 27.9.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLLIBRARY__
#define __SYSTEMLOADER_SLLIBRARY__ 1

#include <Corona-X.h>
#include <SystemLoader/SystemLoader.h>

#if kCXBootloaderCode
    SLPrivate OSNoReturn void SLLeave(SLStatus status);
    OSPrivate OSAddress SLGetMainImageHandle(void);
    OSPrivate OSNoReturn void SLUnrecoverableError(void);

    #if kCXTargetOSApple
        #define SLPrintError SLPrintString

        OSPrivate void SLPrintString(const char *s, ...);
    #else /* !kCXTargetOSApple */
        OSPrivate void SLPrintError(const char *s, ...);
    #endif /* kCXTargetOSApple */

    OSExport OSAddress gSLFirmwareReturnAddress;
    OSExport SLSystemTable *gSLLoaderSystemTable;
    OSExport OSAddress gSLLoaderImageHandle;
    OSExport bool gSLBootServicesEnabled;

    #if kCXBuildDev
        OSPrivate bool SLPromptUser(const char *s, SLSerialPort port);
        OSPrivate void SLShowDelay(const char *s, UInt64 seconds);

        OSPrivate void SLDumpProcessorState(bool standard, bool system, bool debug);
        OSPrivate void SLPrintBasicState(CXKProcessorBasicState *state);
        OSPrivate void SLPrintSystemState(CXKProcessorSystemState *state);
        OSPrivate void SLPrintDebugState(CXKProcessorDebugState *state);

        OSExport OSSize gSLLoaderImageSize;
    #else /* !kCXBuildDev */
        #define SLPrintString(s, ...) do {} while(0)
    #endif /* kCXBuildDev */

    #if kCXDebug
        OSPrivate void __SLSerialConsoleInitAll(void);
        OSPrivate void __SLVideoConsoleInitAll(void);
        OSPrivate void __SLBitmapFontInitialize(void);
        OSPrivate void __SLLibraryInitialize(void);
    #endif /* kCXDebug */
#endif /* kCXBootloaderCode */

extern void SLPS(const char *s, ...);

#endif /* !defined(__SYSTEMLOADER_SLLIBRARY__) */
