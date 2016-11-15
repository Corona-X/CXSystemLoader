/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLRuntimeServices.h - EFI Runtime Services Declarations         */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:45 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:45 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLRUNTIMESERVICES__
#define __SYSTEMLOADER_SLRUNTIMESERVICES__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

typedef struct {
    SLTableHeader header;
    OSAddress unused[14];
} SLRuntimeServices;

#if kCXBootloaderCode
    OSPrivate SLRuntimeServices *SLRuntimeServicesGetCurrent(void);
#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLRUNTIMESERVICES__) */
