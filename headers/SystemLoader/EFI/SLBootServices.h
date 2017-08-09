/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBootServices.h - EFI Boot Services Declarations               */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:15 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLBOOTSERVICES__
#define __SYSTEMLOADER_EFI_SLBOOTSERVICES__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode

typedef OSEnum(UInt32) {
    kSLSearchTypeEverything         = 0,
    kSLSearchTypeNextRegistered     = 1,
    kSLSearchTypeByProtocol         = 2
} SLSearchType;

typedef OSEnum(UInt32) {
    kSLAllocTypeAnyPages            = 0,
    kSLAllocTypeMaxAddress          = 1,
    kSLAllocTypeAtAddress           = 2
} SLAllocationType;

typedef OSEnum(UInt32) {
    kSLMemoryTypeReserved           = 0x0,
    kSLMemoryTypeLoaderCode         = 0x1,
    kSLMemoryTypeLoaderData         = 0x2,
    kSLMemoryTypeBootCode           = 0x3,
    kSLMemoryTypeBootData           = 0x4,
    kSLMemoryTypeRuntimeCode        = 0x5,
    kSLMemoryTypeRuntimeData        = 0x6,
    kSLMemoryTypeFree               = 0x7,
    kSLMemoryTypeUnusable           = 0x8,
    kSLMemoryTypeACPIReclaim        = 0x9,
    kSLMemoryTypeACPINVS            = 0xA,
    kSLMemoryTypeMappedIO           = 0xB,
    kSLMemoryTypeMappedIOPorts      = 0xC,
    kSLMemoryTypePALCode            = 0xD,
    kSLMemorytypePersistent         = 0xE
} SLMemoryType;

typedef struct {
    UInt32              entryType;
    OSAddress           physicalAddress;
    OSAddress           virtualAddress;
    OSCount             pageCount;
    UInt64              attributes;
    UInt64              padding;
} SLMemoryDescriptor;

typedef struct {
    UInt64              key;
    OSCount             entryCount;
    SLMemoryDescriptor  *entries;
} SLMemoryMap;

typedef struct {
    SLTableHeader header;
    OSAddress raiseTPL;
    OSAddress restoreTPL;

    SLABI SLStatus (*allocatePages)(SLAllocationType allocType, SLMemoryType type, OSCount pageCount, OSAddress *address);
    SLABI SLStatus (*freePages)(OSAddress address, OSCount pageCount);
    SLABI SLStatus (*getMemoryMap)(OSSize *mapSize, SLMemoryDescriptor *map, UInt64 *key, OSSize *descriptorSize, UInt32 *version);
    SLABI SLStatus (*allocate)(SLMemoryType type, OSSize size, OSAddress *address);
    SLABI SLStatus (*free)(OSAddress address);

    OSAddress createEvent;
    OSAddress setTimer;
    OSAddress waitForEvent;
    OSAddress signalEvent;
    OSAddress closeEvent;
    OSAddress checkEvent;

    OSAddress installProtocol;
    OSAddress reinstallProtocol;
    OSAddress uninstallProtocol;
    SLABI SLStatus (*handleProtocol)(OSAddress handle, SLProtocol *protocol, OSAddress *interface);
    OSAddress reserved;
    OSAddress registerProtocolNotify;
    SLABI SLStatus (*locateHandle)(SLSearchType type, SLProtocol *protocol, OSAddress key, UInt64 *bufferSize, OSAddress buffer);
    OSAddress locateDevicePath;
    OSAddress installConfigTable;

    OSAddress loadImage;
    OSAddress startImage;
    SLABI SLStatus (*exit)(OSAddress imageHandle, SLStatus exitStatus, OSSize reasonSize, OSUTF16Char *reason);
    OSAddress unloadImage;
    SLABI SLStatus (*terminate)(OSAddress imageHandle, UInt64 memoryMapKey);

    OSAddress getNextMonotonicCount;
    SLABI SLStatus (*stall)(UInt64 microseconds);
    OSAddress setWatchdogTimer;

    OSAddress connectController;
    OSAddress disconnectController;

    OSAddress openProtocol;
    OSAddress closeProtocol;
    OSAddress openProtocolInfo;

    OSAddress protocolsPerHandle;
    SLABI SLStatus (*localeHandles)(SLSearchType type, SLProtocol *protocol, OSAddress key, OSCount *count, OSAddress **buffer);
    OSAddress locateProtocol;
    OSAddress installProtocolInterfaces;
    OSAddress uninstallProtocolInterfaces;

    OSAddress calculateCRC32;
    OSAddress copyMemory;
    OSAddress setMemory;
    OSAddress createEventExtended;
} SLBootServices;

#if kCXBootloaderCode
    OSPrivate SLBootServices *SLBootServicesGetCurrent(void);

    OSPrivate bool SLBootServicesAllocatePages(OSAddress base, OSCount pages);
    OSPrivate OSAddress SLBootServicesAllocateAnyPages(OSCount pages);
    OSPrivate bool SLBootServicesFreePages(OSAddress base, OSCount pages);
    OSPrivate OSAddress SLBootServicesAllocate(OSSize size);
    OSPrivate bool SLBootServicesFree(OSAddress address);

    OSPrivate SLMemoryMap *SLBootServicesGetMemoryMap(void);
    OSPrivate SLMemoryMap *SLBootServicesTerminate(void);
    OSPrivate bool SLBootServicesHaveTerminated(void);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLBOOTSERVICES__) */
