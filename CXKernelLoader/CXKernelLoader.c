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

// Kill boot services
// Setup IDT
// Read SMBIOS
// Setup ACPI
// Determine Memory Size
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

        bool isAvailable;
        bool isCorrupt;
    } *zones;

    SLMemoryMap *bootloaderMap;
    OSCount zoneCount;

    OSSize availableSize;
    OSSize corruptSize;

    OSSize fullSize;
} SLMemoryZoneInfo;

typedef struct __SLMemoryZone SLMemoryZone;

OSPrivate SLMemoryZoneInfo *SLReadMemoryMap(SLMemoryMap *map);
OSPrivate SLMemoryZoneInfo *SLMemoryZoneRead(SLMemoryMap *map);

OSPrivate void SLDumpMemoryInfo(SLMemoryZoneInfo *info);
OSPrivate void SLDumpMemoryMap(SLMemoryMap *map);

SLMemoryZoneInfo *SLReadMemoryMap(SLMemoryMap *map)
{
    SLMemoryZoneInfo *info = SLAllocate(sizeof(SLMemoryZoneInfo));

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

    for (OSIndex i = 0; ((OSCount)i) < map->entryCount; i++)
    {
        SLMemoryDescriptor *descriptor = &map->entries[i];

        OSSize length = descriptor->pageCount << kSLBootPageShift;
        OSIndex zoneIndex;

        bool isAvailable;
        bool isCorrupt;

        if (!descriptor->pageCount)
        {
            SLPrintString("Warning: Empty descriptor (%u)\n", i);
            continue;
        }

        if (descriptor->entryType == kSLMemoryTypeUnusable)
            isCorrupt = true;
        else
            isCorrupt = false;

        switch (descriptor->entryType)
        {
            case kSLMemoryTypeReserved:
                SLPrintString("Warning: Reserved memory type (%u, %u pages)\n", i, descriptor->pageCount);

                isAvailable = false;
            break;
            case kSLMemoryTypeLoaderCode:
            case kSLMemoryTypeLoaderData:
            case kSLMemoryTypeBootCode:
            case kSLMemoryTypeBootData:
                isAvailable = true;
            break;
            case kSLMemoryTypeRuntimeCode:
            case kSLMemoryTypeRuntimeData:
                isAvailable = false;
            break;
            case kSLMemoryTypeFree:
            case kSLMemoryTypeACPIReclaim:
            case kSLMemoryTypeUnusable:
                isAvailable = true;
            break;
            case kSLMemoryTypeACPINVS:
            case kSLMemoryTypeMappedIO:
            case kSLMemoryTypeMappedIOPorts:
            case kSLMemoryTypePALCode:
            case kSLMemorytypePersistent:
                isAvailable = false;
            break;
            default:
                SLPrintString("Warning: Invalid memory type (%u)\n", i);
                continue;
        }

        for (zoneIndex = 0; ; zoneIndex++)
        {
            SLMemoryZone *zone = &info->zones[zoneIndex];
            if (!zone->length) break;

            if (zone->isAvailable != isAvailable || zone->isCorrupt != isCorrupt)
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

            zone->isAvailable = isAvailable;
            zone->isCorrupt = isCorrupt;

            info->zoneCount++;
        }

        if (isCorrupt)
            info->corruptSize += length;

        if (isAvailable)
            info->availableSize += length;

        info->fullSize += length;
    }

    OSAddress top = info->zones[info->zoneCount - 1].end;

    if (info->fullSize < (OSPointerValue)top)
        SLPrintString("Warning: Memory map has holes!\n");

    return info;
}

SLMemoryZoneInfo *SLMemoryZoneRead(SLMemoryMap *map)
{
    if (!map)
    {
        SLPrintString("No map\n");
        return kOSNullPointer;
    }

    SLMemoryZoneInfo *info = SLReadMemoryMap(map);

    if (!info)
    {
        SLPrintString("Couldn't read map.\n");

        SLFree(map->entries);
        SLFree(map);

        return kOSNullPointer;
    }

    SLPrintString("Memory zone count: %zu\n", info->zoneCount);
    SLPrintString("Detected memory size: %zu\n", info->fullSize);
    SLPrintString("Available size: %zu\n", info->availableSize);
    SLPrintString("Corrupted size: %zu\n", info->corruptSize);

    SLDumpMemoryInfo(info);

    return info;
}

void SLDumpMemoryInfo(SLMemoryZoneInfo *info)
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

        SLPrintString("%02u: 0x%08X --> 0x%08X (%s%s)\n", i, start, end,
            (info->zones[i].isAvailable ? "available" : ""),
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

    SLMemoryMap *map = SLBootServicesTerminate();
    SLMemoryZoneRead(map);

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
