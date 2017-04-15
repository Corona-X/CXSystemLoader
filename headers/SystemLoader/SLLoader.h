/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLoader.h - System Loader Function definitions                 */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 26.9.2016  - 10:00 PM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLLOADER__
#define __SYSTEMLOADER_SLLOADER__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLSystemTable.h>

#define kSLLoaderVersionString   "A.1"
#define kSLLoaderBuildString     "0040"

#define kSLLoaderWelcomeString   "Corona-X System Loader Version " kSLLoaderVersionString " [Build " kSLLoaderBuildString "]\r\n"
#define kSLLoaderDataDirectory   "/EFI/corona"
#define kSLLoaderConfigFile      "SLConfigFile"

#if kCXBootloaderCode && !kCXAssemblyCode
    OSPrivate SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable);

    #if kCXBuildDev
        OSPrivate void SLRunTests(void);
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode && !kCXAssemblyCode*/

#endif /* !defined(__SYSTEMLOADER_SLLOADER__) */
