/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLRuntimeServices.h - EFI Runtime Services Declarations         */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:45 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLRUNTIMESERVICES__
#define __SYSTEMLOADER_EFI_SLRUNTIMESERVICES__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode

typedef struct {
    SLTableHeader header;
    OSAddress getTime;
    OSAddress setTime;
    OSAddress getWakeupTime;
    OSAddress setWakeupTime;
    OSAddress setVirtualMap;
    OSAddress convertPointer;
    SLABI SLStatus (*getVariable)(OSUTF16Char *name, SLProtocol *vendorUID, UInt32 *attributes, UIntN *size, OSAddress data);
    OSAddress getNextVariableName;
    OSAddress setVariable;
    OSAddress getCounterHigh;
    OSAddress resetSystem;
    OSAddress updateCapsule;
    OSAddress queryCapsuleCapabilities;
    OSAddress quearyVariableInfo;
} SLRuntimeServices;

#if kCXBootloaderCode
    OSPrivate SLRuntimeServices *SLRuntimeServicesGetCurrent(void);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLRUNTIMESERVICES__) */
