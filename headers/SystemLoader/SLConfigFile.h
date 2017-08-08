/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLConfigFile.h - Routines for Loading and Applying SL Config    */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 14.10.2016 - 6:00 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLCONFIGFILE__
#define __SYSTEMLOADER_SLCONFIGFILE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <System/OSUID.h>

#if !kCXAssemblyCode

typedef struct {
    OSUIDIntelData rootParitionID;
    OSUTF16Char *path;
} SLConfigFile;

#if kCXBootloaderCode
    OSPrivate SLConfigFile *SLConfigLoad(OSUTF8Char *path);
    OSPrivate SLConfigFile *SLConfigGet(void);
    OSPrivate bool SLConfigSave(SLConfigFile *config);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLCONFIGFILE__) */
