#include <SystemLoader/Kernel/SLKernelLoader.h>
#include <SystemLoader/Kernel/SLSerialConsole.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLMach-O.h>
#include <SystemLoader/SLBase.h>

#include <Kernel/Shared/XKLegacy.h>
#include <Kernel/C/XKUnicode.h>
#include <Kernel/C/XKMemory.h>

#define kCXLowMemoryString "Corona System " kCXSystemName " " kCXSystemRevision "." kCXSystemMajorVersion ""

// Setup IDT
// Read SMBIOS
// Setup ACPI
// Determine Memory Size
// Kill boot services
// Map Memory in CR3
// Map runtime services high
// Update runtime services map
// Setup IOAPIC/Multithreading
// Load Kernel + Servers
// Init Timing
// Init real VM
// Init logging
// Start other CPUs
// Trampoline through Kernel into SystemServer

OSPrivate void SLSetupMem0(void);

typedef struct {
    struct __SLMemoryZone {
        OSAddress physical;
        OSAddress virtual;
        OSAddress end;

        OSSize length;

        bool isCorrupt;
        bool isBoot;
        bool isFree;
    } *zones;

    SLMemoryMap *bootloaderMap;
    OSCount zoneCount;

    OSSize availableSize;
    OSSize corruptSize;

    OSSize bootUsed;
    OSSize fullSize;
} SLMemoryInfo;

typedef struct __SLMemoryZone SLMemoryZone;

OSPrivate SLMemoryInfo *SLReadMemoryMap(SLMemoryMap *map);
OSPrivate void SLSetupMemoryInfo(void);

OSPrivate void SLDumpMemoryInfo(SLMemoryInfo *info);
OSPrivate void SLDumpMemoryMap(SLMemoryMap *map);

SLMemoryInfo *SLReadMemoryMap(SLMemoryMap *map)
{
    SLMemoryInfo *info = SLAllocate(sizeof(SLMemoryInfo));

    if (!info)
    {
        SLPrintString("Allocator Error.\n");
        return kOSNullPointer;
    }

    info->zones = SLAllocate(map->entryCount * sizeof(SLMemoryZone));
    info->bootloaderMap = map;

    if (!info->zones || !info)
    {
        SLPrintString("Allocator Error.\n");
        SLFree(info);

        return kOSNullPointer;
    }

    XKMemoryZero(info->zones, map->entryCount * sizeof(SLMemoryZone));
    OSIndex lastRegion = 0;

    for (OSIndex i = 0; ((OSCount)i) < map->entryCount; i++)
    {
        SLMemoryDescriptor *descriptor = &map->entries[i];
        OSSize length = descriptor->pageCount << kSLBootPageShift;
        OSIndex zoneIndex;
        bool isReserved;
        bool isCorrupt;
        bool isBoot;

        if (!descriptor->pageCount) {
            SLPrintString("Warning: Empty descriptor (%u)\n", i);
            continue;
        } else {
            lastRegion = i;
        }

        if (descriptor->entryType == kSLMemoryTypeUnusable)
            isCorrupt = true;
        else
            isCorrupt = false;

        switch (descriptor->entryType)
        {
            case kSLMemoryTypeReserved:
                SLPrintString("Warning: Reserved memory type (%u, %u pages)\n", i, descriptor->pageCount);

                isReserved = true;
                isBoot = false;
            break;
            case kSLMemoryTypeLoaderCode:
            case kSLMemoryTypeLoaderData:
            case kSLMemoryTypeBootCode:
            case kSLMemoryTypeBootData:
                isReserved = false;
                isBoot = true;
            break;
            case kSLMemoryTypeRuntimeCode:
            case kSLMemoryTypeRuntimeData:
                isReserved = true;
                isBoot = false;
            break;
            case kSLMemoryTypeFree:
                isReserved = false;
                isBoot = false;
            break;
            case kSLMemoryTypeUnusable:
                isReserved = false;
                isBoot = false;
            break;
            case kSLMemoryTypeACPIReclaim:
                isReserved = false;
                isBoot = true;
            break;
            case kSLMemoryTypeACPINVS:
            case kSLMemoryTypeMappedIO:
            case kSLMemoryTypeMappedIOPorts:
            case kSLMemoryTypePALCode:
            case kSLMemorytypePersistent:
                isReserved = true;
                isBoot = false;
            break;
            default:
                SLPrintString("Warning: Invalid memory type (%u)\n", i);
                continue;
        }

        for (zoneIndex = 0; ; zoneIndex++)
        {
            SLMemoryZone *zone = &info->zones[zoneIndex];
            if (!zone->length) break;

            if (zone->isBoot != isBoot || zone->isFree != !isReserved || zone->isCorrupt != isCorrupt)
                continue;

            if (zone->end == descriptor->physicalAddress)
            {
                zone->length += length;
                zone->end = zone->physical + zone->length;

                break;
            }
        }

        if (!info->zones[zoneIndex].length)
        {
            SLMemoryZone *zone = &info->zones[zoneIndex];
            zone->physical = descriptor->physicalAddress;
            zone->virtual = descriptor->virtualAddress;
            zone->length = length;
            zone->end = zone->physical + zone->length;

            zone->isCorrupt = isCorrupt;
            zone->isFree = !isReserved;
            zone->isBoot = isBoot;

            info->zoneCount++;
        }

        if (isBoot)
            info->bootUsed += length;

        if (isCorrupt)
            info->corruptSize += length;

        if (!isReserved)
            info->availableSize += length;

        info->fullSize += length;
    }

    OSAddress top = info->zones[lastRegion].end;
    SLPrintString("Last Region: %u\n", lastRegion);
    SLPrintString("Final Pointer: %p, Counted Memory: %p\n", top, info->fullSize);

    if (info->fullSize < (OSPointerValue)top)
    {
        SLPrintString("Warning: Memory map has holes!\n");
        SLPrintString("Adjusting max memory from %p --> %p\n", info->fullSize, top);

        info->fullSize = (OSPointerValue)top;
    }

    return info;
}

void SLSetupMemoryInfo(void)
{
    SLMemoryMap *map = SLBootServicesGetMemoryMap();

    if (!map)
    {
        SLPrintString("No map\n");
        return;
    }

    SLMemoryInfo *info = SLReadMemoryMap(map);

    if (!info)
    {
        SLPrintString("Couldn't read map.\n");

        SLFree(map->entries);
        SLFree(map);

        return;
    }

    SLPrintString("Memory zone count: %zu\n", info->zoneCount);
    SLPrintString("Detected memory size: %zu\n", info->fullSize);
    SLPrintString("Available size: %zu\n", info->availableSize);
    SLPrintString("Bootloader memory size: %zu\n", info->bootUsed);
    SLPrintString("Corrupted size: %zu\n", info->corruptSize);

    SLDumpMemoryInfo(info);

    SLFree(info->bootloaderMap->entries);
    SLFree(info->bootloaderMap);

    SLFree(info->zones);
    SLFree(info);
}

void SLDumpMemoryInfo(SLMemoryInfo *info)
{
    if (!info)
    {
        SLPrintString("No info.\n");
        return;
    }

    for (OSIndex i = 0; ((OSCount)i) < info->zoneCount; i++)
    {
        OSAddress start = info->zones[i].physical;
        OSAddress end = info->zones[i].end;

        SLPrintString("%02u: 0x%08X --> 0x%08X (%s%s%s)\n", i, start, end,
            (info->zones[i].isBoot ? "boot, "     : ""),
            (info->zones[i].isFree ? "free"     : ""),
            (info->zones[i].isCorrupt ? "corrupt" : "")
        );
    }
}

void SLDumpMemoryMap(SLMemoryMap *map)
{
    if (!map)
    {
        SLPrintString("No map.\n");
        return;
    }

    for (OSIndex i = 0; ((OSCount)i) < map->entryCount; i++)
    {
        OSAddress start = map->entries[i].physicalAddress;
        OSAddress end = start + (map->entries[i].pageCount << kSLBootPageShift);
        OSAddress virtual = map->entries[i].virtualAddress;

        UInt64 attributes = map->entries[i].attributes;
        const char *type;

        switch (map->entries[i].entryType)
        {
            case kSLMemoryTypeReserved:      type = "Reserved"; break;
            case kSLMemoryTypeLoaderCode:    type = "Loader Code"; break;
            case kSLMemoryTypeLoaderData:    type = "Loader Data"; break;
            case kSLMemoryTypeBootCode:      type = "Boot Code"; break;
            case kSLMemoryTypeBootData:      type = "Boot Data"; break;
            case kSLMemoryTypeRuntimeCode:   type = "Runtime Code"; break;
            case kSLMemoryTypeRuntimeData:   type = "Runtime Data"; break;
            case kSLMemoryTypeFree:          type = "Free"; break;
            case kSLMemoryTypeUnusable:      type = "Unusable"; break;
            case kSLMemoryTypeACPIReclaim:   type = "ACPI Reclaimable"; break;
            case kSLMemoryTypeACPINVS:       type = "ACPI NVS"; break;
            case kSLMemoryTypeMappedIO:      type = "Memory Mapped I/O"; break;
            case kSLMemoryTypeMappedIOPorts: type = "Memory Papped I/O Ports"; break;
            case kSLMemoryTypePALCode:       type = "PAL Code"; break;
            case kSLMemorytypePersistent:    type = "Persistent"; break;
            default: type = "<Error>"; break;
        }

        SLPrintString("%02u: 0x%08X --> 0x%08X [%p] (0x%02X, %s)\n", i, start, end, virtual, attributes, type);
    }
}

OSPrivate void SLSetupVideo(void);

// This function sets up the first KiB in memory.
// We copy '0xC1' to NULL, stamp our OS version right after,
// and then fill the rest of the page with zeros.
void SLSetupMem0(void)
{
    UInt8 mem0[0x400];

    XKMemorySetValue(mem0, 0x400, 0);
    mem0[0] = 0xC1;

    XKMemoryCopy(kCXLowMemoryString, &mem0[1], __builtin_strlen(kCXLowMemoryString));
    XKMemoryCopy(mem0, kOSNullPointer, 0x400);
}

void CXKernelLoaderMain(OSUnused SLMachOFile *loadedImage)
{
    // This call is already made in this binary once control is passed here...
    //SLConsoleInitialize();

    if (kCXBuildDev)
    {
        SLPrintString("Entered CXKernelLoader!\n");
        SLPrintString("BootX.car loaded at %p\n", gSLBootXAddress);
    }

    // Set First KiB of RAM to System Info (Below legacy BIOS data area)
    SLSetupMem0();

    if (kCXBuildDev)
        SLPrintString("System info in first KiB: %s\n", (OSUTF8Char *)1);

    // Yay make the screen pretty :)
    SLSetupVideo();

    SLMemoryMap *map = SLBootServicesGetMemoryMap();
    SLDumpMemoryMap(map);
    SLFree(map->entries);
    SLFree(map);

    SLSetupMemoryInfo();

    SLSerialConsoleReadKey(true);
    SLLeave(kSLStatusSuccess);
}

OSNoReturn void SLLeave(SLStatus status)
{
    if (gSLBootServicesEnabled)
        SLBootServicesExit(status);
    else
        SLRuntimeServicesResetSystem(kSLResetTypeShutdown, status, kOSNullPointer);
}

#if 0

In this section is pieces of code which have been removed from CXSystemLoader.
They must be reimplemented properly into the kernel loader.

From SLMemoryAllocator.c:

// On allocator initialization:
OSPrivate void SLMemoryAllocatorOnBootServicesTerminate(SLMemoryMap *finalMemoryMap, OSAddress context);

SLBootServicesRegisterTerminationFunction(SLMemoryAllocatorOnBootServicesTerminate, kOSNullPointer);

void SLMemoryAllocatorOnBootServicesTerminate(OSUnused SLMemoryMap *finalMemoryMap, OSUnused OSAddress context)
{
    gSLCurrentHeap.shouldFree = false;
}

From EFI/SLSystemTable.c:

OSPrivate CPRootDescriptor *SLSystemTableGetACPIRoot(SLSystemTable *table);

CPRootDescriptor *SLSystemTableGetACPIRoot(SLSystemTable *table)
{
    CPRootDescriptor *root = kOSNullPointer;
 
    for (OSIndex i = 0; i < table->numberOfConfigTables; i++)
    {
        SLConfigTable *configTable = &table->configTables[i];
        OSUIDIntelData acpiTableID = kSLACPITableID;
 
        if (!XKMemoryCompare(&configTable->id, &acpiTableID, sizeof(OSUIDIntelData)))
            root = configTable->pointer;
    }
 
    if (root && CPRootDescriptorValidate(root)) {
        return root;
    } else {
        return kOSNullPointer;
    }
}

From EFI/SLBootServices.c:

OSPrivate void SLBootServicesRegisterTerminationFunction(void (*function)(SLMemoryMap *finalMap, OSAddress context), OSAddress context);

typedef struct __SLBootServicesTerminateHandler {
    void (*function)(SLMemoryMap *finalMap, OSAddress context);
    OSAddress context;
    struct __SLBootServicesTerminateHandler *next;
} SLBootServicesTerminateHandler;

SLBootServicesTerminateHandler *gSLBootServicesFirstHandler = kOSNullPointer;

void SLBootServicesRegisterTerminationFunction(void (*function)(SLMemoryMap *finalMap, OSAddress context), OSAddress context)
{
    SLBootServicesCheck((void)(0));
    
    SLBootServicesTerminateHandler *newHandler = SLAllocate(sizeof(SLBootServicesTerminateHandler)).address;
    SLBootServicesTerminateHandler *handler = gSLBootServicesFirstHandler;
    
    if (OSExpect(handler)) {
        while (handler->next)
            handler = handler->next;
        
        handler->next = newHandler;
        handler = newHandler;
    } else {
        gSLBootServicesFirstHandler = handler = newHandler;
    }
    
    handler->function = function;
    handler->context = context;
    handler->next = kOSNullPointer;
}

SLBootServicesTerminateHandler *handler = gSLBootServicesFirstHandler;
XKLog(kXKLogLevelInfo, "Calling Boot Services Terminate Handlers...\n");

while (handler)
{
    handler->function(finalMemoryMap, handler->context);
    
    SLBootServicesTerminateHandler *oldHandler = handler;
    handler = handler->next;
    SLFree(oldHandler);
}

XKLog(kXKLogLevelInfo, "All Handlers Called; Terminating Boot Services...");

From EFI/SLFile.c:

OSUTF16Char *SLPathToEFIPath(OSUTF8Char *path)
{
    OSSize copySize = XKUTF8Length(path) + 1;
    OSUTF8Char *copy = SLAllocate(copySize).address;
    XKMemoryCopy(path, copy, copySize);
    
    for (OSIndex i = 0; i < (copySize - 1); i++)
    {
        if (copy[i] == '/')
            copy[i] = '\\';
    }
    
    OSUTF16Char *efiPath = XKUTF8ToUTF16(copy);
    SLFree(copy);
    
    return efiPath;
}

#endif
