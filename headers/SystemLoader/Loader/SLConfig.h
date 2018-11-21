/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLConfig.h - Routines for Loading and Applying SL Config        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 14.10.2016 - 6:00 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_LOADER_SLCONFIGFILE__
#define __SYSTEMLOADER_LOADER_SLCONFIGFILE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <Kernel/Shared/XKBootConfig.h>

#define kSLConfigKeyAutoboot "Autoboot"

#if !kCXAssemblyCode

#if kCXBootloaderCode
    #define kSLConfigMaxFileSize (1 << 12)

    OSPrivate XKBootConfig *SLConfigLoad(OSUTF8Char *path);
    OSPrivate XKBootConfig *SLConfigGetCurrent(void);

    OSPrivate bool SLConfigGetBool(XKBootConfig *config, const OSUTF8Char *key, bool defaultValue);

    OSExport XKBootConfig *gSLCurrentConfig;

    #if kCXBuildDev
        OSPrivate void SLConfigDump(XKBootConfig *config);
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode */

#endif /* kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_LOADER_SLCONFIG__) */
