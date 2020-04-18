#include <SystemLoader/Kernel/SLEarlyMemoryInit.h>
#include <SystemLoader/Kernel/SLSerialConsole.h>
#include <SystemLoader/Kernel/SLKernelLoader.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLLibrary.h>
#include <System/OSByteMacros.h>
#include <Kernel/C/CLMemory.h>

OSPrivate bool SLMemoryZoneInfoSanitize(SLMemoryZoneInfo *zoneInfo);
OSPrivate bool SLMemoryZoneInfoAddBootloaderZones(SLMemoryZoneInfo *zoneInfo);
OSPrivate bool SLCanUseMemoryAt(OSAddress base, OSSize size);

SLMemoryZoneInfo *SLProcessMemoryMap(SLMemoryMap *map)
{
    if (!map)
        return kOSNullPointer;

    SLMemoryZoneInfo *info = SLAllocate(sizeof(SLMemoryZoneInfo));

    if (!info)
    {
        SLDebugPrint("Error: Couldn't allocate memory.\n");
        return kOSNullPointer;
    }

    info->zones = SLAllocate(map->entryCount * sizeof(SLMemoryZone));
    info->bootloaderMap = map;

    if (!info->zones || !info)
    {
        SLDebugPrint("Error: Couldn't allocate memory.\n");
        SLFree(info);

        return kOSNullPointer;
    }

    CLMemoryZero(info->zones, map->entryCount * sizeof(SLMemoryZone));
    OSIndex end = map->entryCount;
    OSIndex i = 0;

    while (map->entries[i].entryType == kSLMemoryTypeUnusable)
    {
        if (i++ == (OSIndex)map->entryCount)
        {
            SLDebugPrint("Error: Whole memory map marked corrupted.\n");
            return kOSNullPointer;
        }
    }

    while (map->entries[end - 1].entryType == kSLMemoryTypeUnusable)
    {
        if ((end--) < 0)
        {
            SLDebugPrint("Error: Whole memory map marked corrupted.\n");
            return kOSNullPointer;
        }
    }

    if (i == (end - 1))
    {
        SLDebugPrint("Error: Whole memory map marked corrupted.\n");
        return kOSNullPointer;
    }

    for ( ; i < end; i++)
    {
        SLMemoryDescriptor *descriptor = &map->entries[i];

        OSSize length = descriptor->pageCount << kSLBootPageShift;
        OSIndex zoneIndex;

        bool isAvailable;
        bool isCorrupt;

        if (!descriptor->pageCount)
        {
            SLDebugPrint("Warning: EFI Memory Map contains empty descriptor (Descriptor %u)\n", i);
            continue;
        }

        if (descriptor->entryType == kSLMemoryTypeUnusable)
            isCorrupt = true;
        else
            isCorrupt = false;

        switch (descriptor->entryType)
        {
            case kSLMemoryTypeReserved:
                SLDebugPrint("Warning: EFI Memory Map contains 'reserved' memory (descriptor %u, %u pages)\n", i, descriptor->pageCount);

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
                SLDebugPrint("Warning: EFI Memory Map contains invalid memory type! (type 0x%X, descriptor %u)\n", descriptor->entryType, i);
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
        SLDebugPrint("Warning: EFI Memory Map contains holes!\n");

    return info;
}

#define adjust(d, v, o)                                                                                                     \
    do {                                                                                                                    \
        newAddress = OSAlign ## d((UInt64)zoneInfo->zones[i].v, kSLBootPageSize);                                           \
        delta = newAddress - zoneInfo->zones[i].v;                                                                          \
                                                                                                                            \
        SLDebugPrint("Note: Adjusting %p --> %p (note: will lose %zu bytes)\n", zoneInfo->zones[i].v, newAddress, delta);   \
        zoneInfo->zones[i].v = newAddress;                                                                                  \
                                                                                                                            \
        zoneInfo->zones[i].length o ## = delta;                                                                                 \
    } while (0)

bool SLMemoryZoneInfoSanitize(SLMemoryZoneInfo *zoneInfo)
{
    OSAddress newAddress;
    OSIndex i = 0;
    UInt64 delta;

    if (((UInt64)zoneInfo->zones[i].physical) & kSLBootPageMask)
    {
        SLDebugPrint("Warning: First memory zone not properly page aligned!\n");

        if (!zoneInfo->zones[i].isAvailable) {
            SLDebugPrint("Error: Memory zone is not available to the OS!\n");
            return false;
        } else {
            adjust(Upward, physical, -);

            // We only lose size overall here or in the next case
            zoneInfo->availableSize -= delta;
        }
    }

    i = zoneInfo->zoneCount;

    if (((UInt64)zoneInfo->zones[i].end) & kSLBootPageMask)
    {
        SLDebugPrint("Warning: End of final memory zone not properly page aligned!\n");

        if (!zoneInfo->zones[i].isAvailable) {
            SLDebugPrint("Error: Memory zone is not available to the OS!\n");
            return false;
        } else {
            adjust(Down, end, -);

            // We only lose size overall here or in the previous case
            zoneInfo->availableSize -= delta;
        }
    }

    // Go forward. We have to go back in the case of memory holes.
    for (i = 1; i < (OSIndex)(zoneInfo->zoneCount); i++)
    {
        if (((UInt64)zoneInfo->zones[i].physical) & kSLBootPageMask)
        {
            SLDebugPrint("Warning: Memory zone %zu is not properly page aligned!\n", i);

            if (!zoneInfo->zones[i].isAvailable) {
                if (zoneInfo->zones[i - 1].end != zoneInfo->zones[i].physical)
                {
                    // There's a memory hole before this zone.
                    // We can't fix the misalignment...

                    SLDebugPrint("Error: Memory zone %zu misaligned on a memory hole!\n");
                    SLDebugPrint("Note: This zone is marked as unavailable, aborting.\n");

                    return false;
                }

                // Move head down into previous zone
                adjust(Down, physical, +);

                // We have to fix the previous zone's descriptor
                zoneInfo->zones[i - 1].end = zoneInfo->zones[i].physical;
                zoneInfo->zones[i - 1].length -= delta;
            } else {
                bool shouldFixPrevious = (zoneInfo->zones[i - 1].end == zoneInfo->zones[i].physical);

                // Move head up inside this zone
                adjust(Upward, physical, -);

                if (shouldFixPrevious) {
                    // Fix previous descriptor
                    zoneInfo->zones[i - 1].end = zoneInfo->zones[i].physical;
                    zoneInfo->zones[i - 1].length += delta;
                } else {
                    // The memory we knocked off is in a hole anyway. We simply lose 'delta' bites into the nothingness
                    zoneInfo->availableSize -= delta;
                }
            }
        }
    }

    // End pass. Somewhat necessary.
    for (i = 0; i < (OSIndex)(zoneInfo->zoneCount - 1); i++)
    {
        // This will only be triggered if we have memory holes.
        // Luckily, it's fast if there isn't anything to adjust.
        if (((UInt64)zoneInfo->zones[i].end) & kSLBootPageMask)
        {
            SLDebugPrint("Warning: End of memory zone %zu is not properly page aligned!\n", i);

            if (!zoneInfo->zones[i].isAvailable) {
                if (zoneInfo->zones[i + 1].physical != zoneInfo->zones[i].end)
                {
                    // There's a memory hole before this zone.
                    // We can't fix the misalignment...

                    SLDebugPrint("Error: End of memory zone %zu misaligned on a memory hole!\n");
                    SLDebugPrint("Note: This zone is marked as unavailable, aborting.\n");

                    return false;
                }

                // Move end up into next zone
                adjust(Upward, end, +);

                // We have to fix the next zone's descriptor
                zoneInfo->zones[i - 1].end = zoneInfo->zones[i].physical;
                zoneInfo->zones[i - 1].length -= delta;
            } else {
                bool shouldFixNext = (zoneInfo->zones[i - 1].end == zoneInfo->zones[i].physical);

                // Move end down inside this zone
                adjust(Down, end, -);

                if (shouldFixNext) {
                    // Fix next descriptor
                    zoneInfo->zones[i + 1].physical = zoneInfo->zones[i].end;
                    zoneInfo->zones[i + 1].length += delta;
                } else {
                    // The memory we knocked off is in a hole anyway. We simply lose 'delta' bites into the nothingness
                    zoneInfo->availableSize -= delta;
                }
            }
        }
    }

    // Check if we removed any zones
    // Also check if VM is set at all
    for (i = 0; i < (OSIndex)(zoneInfo->zoneCount); i++)
    {
        if (!zoneInfo->zones[i].length)
        {
            // We got rid of a zone. Copy other zones over it.
            SLDebugPrint("Note: Removing memory zone %zu.\n", i);

            // This isn't the most efficient thing in the world, but it's an edge case so it's alright...
            CLMemoryCopy(&zoneInfo->zones[i + 1], &zoneInfo->zones[i], ((zoneInfo->zoneCount - 1) - i) * sizeof(SLMemoryZone));

            zoneInfo->zoneCount--;
        }

        if (zoneInfo->zones[i].virtual)
        {
            SLDebugPrint("Warning: Virtual address set for zone %zu...\n");

            // This is weird. A virtual map shouldn't have been set yet.
        }
    }

    if (zoneInfo->availableSize & kSLBootPageMask)
    {
        SLDebugPrint("Error: Available memory is not a multiple of page size!\n");
        return false;
    }

    return true;
}

#undef adjust

// This can be optimized as follows:
// Basically, we have 3 blocks of memory to record in the zone map. Our program, our heap, and BootX.car in memory.
// We use an AllocPages from UEFI Boot Services to allocate all of this memory. As such, it's somewhat likely all 3 objects are contiguous in system memory.
// SO, it would be useful to first check for this property and sort our objects into 'object groups' of some sort. We will have 1-3 groups.
// Then, perform the same algorithm as below, except using the contiguous groups instread. This is a better strategy.
bool SLMemoryZoneInfoAddBootloaderZones(SLMemoryZoneInfo *zoneInfo)
{
    // We need at most 6 extra zones. Just allocate and copy for them all... (one per object, one extra in case we split preexisting zones (which will likely happen))
    zoneInfo->zones = SLReallocate(zoneInfo->zones, (zoneInfo->zoneCount + 6) * sizeof(SLMemoryZone));
    OSIndex zone = 0;

    OSAddress heapBegin = SLMemoryAllocatorGetHeapAddress();
    OSLength heapLength = SLMemoryAllocatorGetHeapSize();
    OSAddress heapEnd = heapBegin + heapLength;

    while (zoneInfo->zones[zone].end < heapBegin)
    {
        if (zone++ == (OSIndex)zoneInfo->zoneCount)
        {
            SLDebugPrint("Error: Program heap appears to be past end of memory map!\n");
            return false;
        }
    }

    if (heapEnd > zoneInfo->zones[zone].end)
    {
        SLDebugPrint("Error: Program heap crosses the end of memory zone %zu!\n", zone);
        return false;
    }

    if (!zoneInfo->zones[zone].isAvailable || zoneInfo->zones[zone].isCorrupt)
    {
        SLDebugPrint("Error: Program heap in unavailable zone %zu!\n", zone);
        return false;
    }

    if (heapBegin == zoneInfo->zones[zone].physical) {
        if (heapEnd == zoneInfo->zones[zone].end) {
            // Wow it's perfect. Simply claim the zone as ours.
            zoneInfo->zones[zone].bootloader = true;
        } else {
            // Heap is at the start of the zone. There is more memory past heap end
        }
    } else if (heapEnd == zoneInfo->zones[zone].end) {
        // Heap is at the end of the zone. There is open memory before the heap begins.
    } else {
        // Heap is sorta just hangin out somewhere. Split into three separate zones.
    }

    SLDebugPrint("Heap in zone %zu\n", zone);

    //OSAddress programBegin = gSLMachOFileSelf->loadedSize - gSLMachOFileSelf->stackSize;
    //OSLength programLength = gSLMachOFileSelf->loadedSize + gSLMachOFileSelf->stackSize;
    //OSAddress programEnd = programBegin + programLength;

    // Boot-X

    return true;
}

bool SLCanUseMemoryAt(OSAddress base, OSSize size)
{
    OSAddress rangeEnd = base + size;
    OSIndex zone = 0;

    while (gSLSystemZoneInfo->zones[zone].end < base)
    {
        if (zone++ == (OSIndex)gSLSystemZoneInfo->zoneCount)
            return false; // We don't have this much memory...
    }

    if (rangeEnd > gSLSystemZoneInfo->zones[zone].end)
        return false; // Skipped over zone boundary

    // Otherwise, it's inside a zone. Just make sure the zone is clean and approve the usage...
    return  (gSLSystemZoneInfo->zones[zone].isAvailable && !gSLSystemZoneInfo->zones[zone].bootloader);
}

bool SLDoEarlyMemoryInit(SLMemoryMap *map)
{
    if (!map)
    {
        SLDebugPrint("Error: EFI boot service terminate error.\n");
        return false;
    }

    SLMemoryZoneInfo *zoneInfo = SLProcessMemoryMap(map);

    if (!zoneInfo)
    {
        SLDebugPrint("Error: EFI memory map not valid.\n");

        SLFree(map->entries);
        SLFree(map);

        return false;
    }

    if (!SLMemoryZoneInfoAddBootloaderZones(zoneInfo))
    {
        SLFree(zoneInfo->zones);
        SLFree(zoneInfo);

        return false;
    }

    if (!SLMemoryZoneInfoSanitize(zoneInfo))
    {
        SLFree(zoneInfo->zones);
        SLFree(zoneInfo);

        return false;
    }

    if (kCXBuildDev)
    {
        SLDumpMemoryZoneInfo(zoneInfo);

        SLPrintString("Note: Other Objects in Memory:\n");

        SLPrintString("0: Program Code:  %p --> %p [%u]\n",
                      gSLMachOFileSelf->loadAddress,
                      gSLMachOFileSelf->loadAddress + gSLMachOFileSelf->loadedSize,
                      gSLMachOFileSelf->loadedSize
                      );

        SLPrintString("1: Program Stack: %p --> %p [%u]\n",
                      gSLMachOFileSelf->stackAddress - gSLMachOFileSelf->stackSize,
                      gSLMachOFileSelf->stackAddress,
                      gSLMachOFileSelf->stackSize
                      );

        SLPrintString("2: Program Heap:  %p --> %p [%u] (Note: Max @ %p)\n",
                      gSLCurrentHeap.baseAddress,
                      gSLCurrentHeap.baseAddress + gSLCurrentHeap.currentSize,
                      gSLCurrentHeap.currentSize,
                      gSLCurrentHeap.baseAddress + gSLCurrentHeap.maxSize
                      );

        SLPrintString("3: BootX.car:     %p --> %p [%u]\n",
                      gSLBootXAddress,
                      gSLBootXAddress + gSLBootXSize,
                      gSLBootXSize
                      );
    }

    // How to handle these objects:
    // --> First, let's make stack contiguous in the mach-o loader.
    // --> Then, this binary + its stack is protected internally but not later
    // --> Same for program heap. We can make a new heap for the kernel and place important kernel stuff on it.
    // --> BootX.car should be moved in RAM somewhere. Where? Maybe right before kernel heap? I dunno...

    // State we want to pass over to the kernel:
    // --> Kernel Stack in memory (lowest page marked as absent)
    // --> Kernel Binary in memory
    // --> Kernel Configuration structure
    // --> Paging Tables

    // Stuff we want to generate anywhere:
    // --> CPU Info structures for each CPU
    // --> Null page with info possibily

    // When in the course of booting a computer system,
    // it becomes necessary for one loader to allocate
    // the memory which it will use for future objects,
    // and to assume among said memory of the system,
    // the pages to which the laws of physical and virtual
    // memory entitle them, a decent loader requires that
    // a page-based allocator should be declared.

    // Put as much RAM in as you want, we're only gonna map so much of it...
    // We're gonna start with the 'bottom 512G'
    // Mainly because it's one entry of PML4 (and it's more than most people have)
    if (kCXBuildDev)
    {
        if (zoneInfo->availableSize > (1L << 39))
        {
            SLPrintString("Note: This system has a ton of memory.\n");
            SLPrintString("Just saying... (also only mapping 512G for now)\n");
        }
    }

    // Why is our memory not a multiple of the sallest page size on this architecture?
    if (zoneInfo->availableSize & 0xFFF)
    {
        // I'm not really sure what to do here...?
        // Uhh... let's just warn for now...
        SLDebugPrint("Note: Free RAM is not a multiple of page size (4096 bytes).\n");
    }

    // Set First page of RAM to System Info (Below legacy BIOS data area)
    if (zoneInfo->zones[0].physical == kOSNullPointer && zoneInfo->zones[0].length >= kSLBootPageSize) {
        if (zoneInfo->zones[0].isAvailable && !zoneInfo->zones[0].bootloader && !zoneInfo->zones[0].isCorrupt) {
            SLSetupMem0();

            SLDebugPrint("System info in first page: %s\n", (OSUTF8Char *)1);
        } else {
            SLDebugPrint("Warning: Memory address '0' marked not free.\n");
        }
    } else {
        SLDebugPrint("Warning: Memory address '0' not present!\n");
    }

    gSLSystemZoneInfo = zoneInfo;

    //

    return true;
}

#if kCXBuildDev

void SLDumpMemoryZoneInfo(SLMemoryZoneInfo *info)
{
    if (!info) return;

    SLPrintString("Memory Zone Info:\n");
    SLPrintString("Zone count: %zu\n", info->zoneCount);
    SLPrintString("Memory Size: %zu\n", info->fullSize);
    SLPrintString("Available Size: %zu\n", info->availableSize);
    SLPrintString("Corrupt Size: %zu\n", info->corruptSize);
    SLPrintString("Zones:\n");

    for (OSIndex i = 0; ((OSCount)i) < info->zoneCount; i++)
    {
        OSAddress start = info->zones[i].physical;
        OSAddress end = info->zones[i].end;

        SLPrintString("%02u: 0x%08X --> 0x%08X", i, start, end);

        if (info->zones[i].isAvailable) SLPrintString(" (available)");
        if (info->zones[i].isCorrupt)   SLPrintString(" (corrupted)");

        SLPrintString("\n");
    }
}

void SLDumpMemoryMap(SLMemoryMap *map)
{
    if (!map) return;

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

#endif /* kCXBuildDev */
