/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLoader.h - System Loader Function definitions                 */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 26.9.2016  - 10:00 PM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 27.9.2016  - 1:45 AM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLLOADER__
#define __SYSTEMLOADER_SLLOADER__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <Kernel/CXKBootOptions.h>
#include <SystemLoader/SLSerial.h>
#include <SystemLoader/SLConfigFile.h>

#define kSLLoaderVersionString   "A.1"
#define kSLLoaderBuildString     "0033"

#define kSLLoaderWelcomeString   "Corona-X System Loader Version " kSLLoaderVersionString " [Build " kSLLoaderBuildString "]\r\n"
#define kSLLoaderDataDirectory   "EFI/corona"
#define kSLLoaderConfigFile      "SLConfigFile"

#if kCXBootloaderCode
    OSPrivate SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable);

    #if kCXBuildDev
        OSPrivate void SLLoaderSerial0OutputUTF8(UInt8 character);
        OSPrivate void SLLoaderSetupSerial(void);

        OSPrivate void SLRunTests(void);
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLLOADER__) */
