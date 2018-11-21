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

typedef OSEnum(UInt32, SLResetType) {
    kSLResetTypeCold                = 0,
    kSLResetTypeWarm                = 1,
    kSLResetTypeShutdown            = 2,
    kSLResetTypePlatformSpecific    = 3
};

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
    SLABI OSNoReturn void (*resetSystem)(SLResetType type, SLStatus status, UIntN dataSize, OSAddress data);
    OSAddress updateCapsule;
    OSAddress queryCapsuleCapabilities;
    OSAddress queryVariableInfo;
} SLRuntimeServices;

OSPrivate SLRuntimeServices *SLRuntimeServicesGetCurrent(void);

OSPrivate OSNoReturn void SLRuntimeServicesResetSystem(SLResetType type, SLStatus status, const OSUTF8Char *reason);

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLRUNTIMESERVICES__) */
