/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBootServices.h - EFI Boot Services Declarations               */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:15 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:15 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBOOTSERVICES__
#define __SYSTEMLOADER_SLBOOTSERVICES__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

typedef enum {
    kSLSearchTypeEverything,
    kSLSearchTypeNextRegistered,
    kSLSearchTypeByProtocol
} SLSearchType;

typedef enum {
    kSLAllocTypeAnyPages,
    kSLAllocTypeMaxAddress,
    kSLAllocTypeAtAddress
} SLAllocType;

typedef enum {
    kSLMemoryTypeReserved,
    kSLMemoryTypeLoaderCode,
    kSLMemoryTypeLoaderData,
    kSLMemoryTypeBootCode,
    kSLMemoryTypeBootData,
    kSLMemoryTypeRuntimeCode,
    kSLMemoryTypeRuntimeData,
    kSLMemoryTypeConventional,
    kSLMemoryTypeUnusable,
    kSLMemoryTypeACPIReclaimed,
    kSLMemoryTypeACPINVS,
    kSLMemoryTypeMappedIO,
    kSLMemoryTypeMappedIOPorts,
    kSLMemoryTypePALCode,
    kSLMemorytypePersistent
} SLMemoryType;

typedef struct {
    UInt16 unused;
    UInt16 keycode;
} SLKeyPress;

typedef struct {
    SLABI SLStatus (*reset)(OSAddress this, bool extendedVerify);
    SLABI SLStatus (*readKey)(OSAddress this, SLKeyPress *key);
    OSAddress unused2;
} SLSimpleTextInput;

typedef struct {
    OSAddress unused1;
    SLABI SLStatus (*printUTF16)(OSAddress this, OSUTF16Char *string);
    OSAddress unused2[8];
} SLSimpleTextOutput;

typedef struct {
    UInt32 entryType;
    UInt32 padding0;
    OSAddress physicalAddress;
    OSAddress virtualAddress;
    UInt64 pageCount;
    UInt64 attributes;
    UInt64 padding1;
} SLMemoryDescriptor;

typedef struct {
    SLTableHeader header;
    OSAddress unused1[2];
    SLABI SLStatus (*allocatePages)(SLAllocType allocType, SLMemoryType type, UIntN pageCount, OSAddress *address);
    OSAddress unused2;
    SLABI SLStatus (*getMemoryMap)(UIntN *mapSize, SLMemoryDescriptor *map, UIntN *key, UIntN *descriptorSize, UInt32 *version);
    SLABI SLStatus (*allocate)(SLMemoryType type, UIntN size, OSAddress *address);
    SLABI SLStatus (*free)(OSAddress address);
    OSAddress unused3[9];
    SLABI SLStatus (*handleProtocol)(OSAddress handle, SLProtocol *protocol, OSAddress *interface);
    OSAddress reserved;
    OSAddress unused4;
    SLABI SLStatus (*locateHandle)(SLSearchType type, SLProtocol *protocol, OSAddress key, UIntN *bufferSize, OSAddress buffer);
    OSAddress unused5[4];
    SLABI SLStatus (*exit)(OSAddress imageHandle, SLStatus exitStatus, UIntN reasonSize, OSUTF16Char *reason);
    OSAddress unused6;
    SLABI SLStatus (*terminate)(OSAddress imageHandle, UIntN memoryMapKey);
    OSAddress unused7;
    SLABI SLStatus (*stall)(UIntN microseconds);
    OSAddress unused8[7];
    SLABI SLStatus (*localeHandles)(SLSearchType type, SLProtocol *protocol, OSAddress key, UIntN *count, OSAddress **buffer);
    OSAddress unused9[7];
} SLBootServices;

#if kCXBootloaderCode
    OSPrivate SLBootServices *SLBootServicesGetCurrent(void);

    OSPrivate bool SLBootServicesAllocatePages(OSAddress base, OSCount pages);
    OSPrivate OSBuffer SLBootServicesAllocateAnyPages(OSCount pages);
    OSPrivate OSBuffer SLBootServicesAllocate(OSSize size);
    OSPrivate bool SLBootServicesFree(OSBuffer buffer);

    OSPrivate CXKMemoryMap *SLBootServicesGetMemoryMap(void);
    OSPrivate CXKMemoryMap *SLBootServicesTerminate(void);
    OSPrivate bool SLBootServicesHaveTerminated(void);

    OSPrivate bool SLDelayProcessor(UIntN time, bool useBootServices);
    OSPrivate char SLWaitForKeyPress(void);

    OSPrivate bool SLBootServicesOutputString(OSUTF16Char *string);
#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLBOOTSERVICES__) */
