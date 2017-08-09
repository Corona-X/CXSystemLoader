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

#define kSLLoaderSystemName         "Corona-X"
#define kSLLoaderVersionString      "A.1"
#define kSLLoaderBuildString        "004B"

#define kSLLoaderWelcomeString      kSLLoaderSystemName " System Loader Version " kSLLoaderVersionString " [Build " kSLLoaderBuildString "]\n"
#define kSLLoaderBootArchivePath    "/boot/BootX.car"
#define kSLLoaderBootDirectory      "/boot/"
#define kSLLoaderDataDirectory      "/EFI/corona"
#define kSLLoaderConfigFile         "SLConfigFile"

#if kCXBootloaderCode && !kCXAssemblyCode
    OSPrivate SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable);
#endif /* kCXBootloaderCode && !kCXAssemblyCode*/

#endif /* !defined(__SYSTEMLOADER_SLLOADER__) */
