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

#define kSLLoaderSystemName      "Corona-X"
#define kSLLoaderVersionString   "A.1"
#define kSLLoaderBuildString     "0047"

#define kSLLoaderWelcomeString      kSLLoaderSystemName " System Loader Version " kSLLoaderVersionString " [Build " kSLLoaderBuildString "]\r\n"
#define kSLLoaderBootArchivePath    "/boot/BootX.car"
#define kSLLoaderDataDirectory      "/EFI/corona"
#define kSLLoaderConfigFile         "SLConfigFile"

#if kCXBootloaderCode && !kCXAssemblyCode
    OSPrivate SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable);

    #if kCXBuildDev
        OSPrivate void SLRunTests(void);
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode && !kCXAssemblyCode*/

#endif /* !defined(__SYSTEMLOADER_SLLOADER__) */
