/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLoader.h - System Loader Function definitions                 */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 26.9.2016  - 10:00 PM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_LOADER_SLLOADER__
#define __SYSTEMLOADER_LOADER_SLLOADER__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLSystemTable.h>

#define kSLLoaderWelcomeString      kCXSystemName " System Loader Version " kSLLoaderVersionString " [" kCXBuildTypeString " Build " kSLLoaderBuildString "]\n"
#define kSLLoaderVersionString      kCXSystemRevision "." kCXSystemMajorVersion
#define kSLLoaderBuildString        "0069"

#define kSLLoaderBootArchivePath    "/boot/BootX.car"
#define kSLLoaderBootDirectory      "/boot/"
#define kSLLoaderDataDirectory      "/EFI/corona"
#define kSLLoaderConfigFile         "CXBootConfig"

#if !kCXAssemblyCode && kCXBootloaderCode

OSPrivate SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable);

#endif /* kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_LOADER_SLLOADER__) */
