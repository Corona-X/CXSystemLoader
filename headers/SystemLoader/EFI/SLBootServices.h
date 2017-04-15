/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBootServices.h - EFI Boot Services Declarations               */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:15 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBOOTSERVICES__
#define __SYSTEMLOADER_SLBOOTSERVICES__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode

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

typedef OSEnum(UInt32) {
    kSLMemoryTypeReserved       = 0x0,
    kSLMemoryTypeLoaderCode     = 0x1,
    kSLMemoryTypeLoaderData     = 0x2,
    kSLMemoryTypeBootCode       = 0x3,
    kSLMemoryTypeBootData       = 0x4,
    kSLMemoryTypeRuntimeCode    = 0x5,
    kSLMemoryTypeRuntimeData    = 0x6,
    kSLMemoryTypeFree           = 0x7,
    kSLMemoryTypeUnusable       = 0x8,
    kSLMemoryTypeACPIReclaim    = 0x9,
    kSLMemoryTypeACPINVS        = 0xA,
    kSLMemoryTypeMappedIO       = 0xB,
    kSLMemoryTypeMappedIOPorts  = 0xC,
    kSLMemoryTypePALCode        = 0xD,
    kSLMemorytypePersistent     = 0xE
} SLMemoryType;

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
    UIntN key;
    OSCount entryCount;
    SLMemoryDescriptor *entries;
} SLMemoryMap;

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
    SLTableHeader header;
    OSAddress unused1[2];
    SLABI SLStatus (*allocatePages)(SLAllocType allocType, SLMemoryType type, UIntN pageCount, OSAddress *address);
    OSAddress unused2;
    SLABI SLStatus (*getMemoryMap)(UIntN *mapSize, SLMemoryDescriptor *map, UIntN *key, UIntN *descriptorSize, UInt32 *version);
    OSAddress unused3;
    SLABI SLStatus (*free)(OSAddress address);
    OSAddress unused4[9];
    SLABI SLStatus (*handleProtocol)(OSAddress handle, SLProtocol *protocol, OSAddress *interface);
    OSAddress reserved;
    OSAddress unused5;
    SLABI SLStatus (*locateHandle)(SLSearchType type, SLProtocol *protocol, OSAddress key, UIntN *bufferSize, OSAddress buffer);
    OSAddress unused6[4];
    SLABI SLStatus (*exit)(OSAddress imageHandle, SLStatus exitStatus, UIntN reasonSize, OSUTF16Char *reason);
    OSAddress unused7;
    SLABI SLStatus (*terminate)(OSAddress imageHandle, UIntN memoryMapKey);
    OSAddress unused8;
    SLABI SLStatus (*stall)(UIntN microseconds);
    OSAddress unused9[7];
    SLABI SLStatus (*localeHandles)(SLSearchType type, SLProtocol *protocol, OSAddress key, UIntN *count, OSAddress **buffer);
    OSAddress unused10[7];
} SLBootServices;

#if kCXBootloaderCode
    OSPrivate SLBootServices *SLBootServicesGetCurrent(void);
    OSPrivate void SLBootServicesRegisterTerminationFunction(void (*function)(SLMemoryMap *finalMap, OSAddress context), OSAddress context);

    OSPrivate bool SLBootServicesAllocatePages(OSAddress base, OSCount pages);
    OSPrivate OSBuffer SLBootServicesAllocateAnyPages(OSCount pages);
    OSPrivate OSBuffer SLBootServicesAllocate(OSSize size);
    OSPrivate bool SLBootServicesFree(OSAddress address);

    OSPrivate SLMemoryMap *SLBootServicesGetMemoryMap(void);
    OSPrivate SLMemoryMap *SLBootServicesTerminate(void);
    OSPrivate bool SLBootServicesHaveTerminated(void);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLBOOTSERVICES__) */
