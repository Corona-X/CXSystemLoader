/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBasicIO.h - Basic I/O for CXSystemLoader                      */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 8.8.2017   - 7:00 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBASICIO__
#define __SYSTEMLOADER_SLBASICIO__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode && kCXBootloaderCode
    #if kCXBuildDev
        #define SLDebugPrint(s, ...) SLPrintString(s, ##__VA_ARGS__)
    #else /* Release */
        #define SLDebugPrint(s, ...) do {} while (0)
    #endif /* kCXBuildDev */

    OSPrivate void SLPrintStringFromList(const OSUTF8Char *format, OSVAList args);
    OSPrivate void SLPrintString(const OSUTF8Char *format, ...);

    OSPrivate UInt64 SLNumberFromString(const OSUTF8Char *string, bool *isSigned, UInt64 defaultValue);

    OSExport bool gSLConsoleIsInitialized;
#endif /* kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLBASICIO__) */
