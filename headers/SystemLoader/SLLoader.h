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

#define kSLLoaderVersionString      kCXSystemRevision "." kCXSystemMajorVersion
#define kSLLoaderBuildString        "005C"

#define kSLLoaderWelcomeString      kCXSystemName " System Loader Version " kSLLoaderVersionString " [" kCXBuildTypeString " Build " kSLLoaderBuildString "]\n"
#define kSLLoaderBootArchivePath    "/boot/BootX.car"
#define kSLLoaderBootDirectory      "/boot/"
#define kSLLoaderDataDirectory      "/EFI/corona"
#define kSLLoaderConfigFile         "SLConfigFile"

#if !kCXAssemblyCode && kCXBootloaderCode

OSPrivate SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable);

#endif /* kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLLOADER__) */
