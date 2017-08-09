#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/C/XKMemory.h>

SLBootServices *SLBootServicesGetCurrent(void)
{
    return SLSystemTableGetCurrent()->bootServices;
}

bool SLBootServicesHaveTerminated(void)
{
    return !gSLBootServicesEnabled;
}

#pragma mark - Allocation Routines

bool SLBootServicesAllocatePages(OSAddress base, OSCount pages)
{
    SLBootServicesCheck(false);
    OSAddress result = base;

    SLStatus status = SLBootServicesGetCurrent()->allocatePages(kSLAllocTypeAtAddress, kSLMemoryTypeLoaderData, pages, &result);
    if (SLStatusIsError(status)) return false;
    if (result != base) return false;

    return base;
}

OSAddress SLBootServicesAllocateAnyPages(OSCount pages)
{
    SLBootServicesCheck(kOSNullPointer);
    OSAddress result;

    SLStatus status = SLBootServicesGetCurrent()->allocatePages(kSLAllocTypeAnyPages, kSLMemoryTypeLoaderData, pages, &result);
    if (SLStatusIsError(status)) return kOSNullPointer;

    return result;
}

bool SLBootServicesFreePages(OSAddress base, OSCount pages)
{
    SLBootServicesCheck(false);

    SLStatus status = SLBootServicesGetCurrent()->freePages(base, pages);
    return !SLStatusIsError(status);
}

OSAddress SLBootServicesAllocate(OSSize size)
{
    SLBootServicesCheck(kOSNullPointer);
    OSAddress result;

    SLStatus status = SLBootServicesGetCurrent()->allocate(kSLMemoryTypeLoaderData, size, &result);
    if (SLStatusIsError(status)) return kOSNullPointer;

    return result;
}

bool SLBootServicesFree(OSAddress address)
{
    SLBootServicesCheck(false);

    SLStatus status = SLBootServicesGetCurrent()->free(address);
    return !SLStatusIsError(status);
}

#pragma mark - Other Routines

SLMemoryMap *SLBootServicesGetMemoryMap(void)
{
    SLBootServicesCheck(kOSNullPointer);

    SLMemoryMap *memoryMap = SLAllocate(sizeof(SLMemoryMap));
    if (!memoryMap) return kOSNullPointer;

    XKMemorySetValue(memoryMap, sizeof(SLMemoryMap), 0);
    OSSize memoryMapSize = 0;
    OSSize entrySize = 0;
    UInt32 version = 0;

    SLStatus status = SLBootServicesGetCurrent()->getMemoryMap(&memoryMapSize, kOSNullPointer, &memoryMap->key, &entrySize, &version);
    if (entrySize != sizeof(SLMemoryDescriptor)) status = kSLStatusWrongSize;
    if (status == kSLStatusBufferTooSmall) status = kSLStatusSuccess;
    if (status == kSLStatusBadArgument) status = kSLStatusSuccess;
    if (version != 1) status = kSLStatusIncompatibleVersion;
    if (SLStatusIsError(status)) goto fail;

    // TODO: Figure out why I did this...
    // Try 3 times for some reason?
    for (OSCount i = 0; i < 3; i++)
    {
        memoryMap->entryCount = (memoryMapSize / sizeof(SLMemoryDescriptor));
        memoryMap->entries = SLAllocate(memoryMapSize);
        if (!memoryMap->entries) goto fail;

        memoryMapSize = SLGetObjectSize(memoryMap->entries);
        status = SLBootServicesGetCurrent()->getMemoryMap(&memoryMapSize, memoryMap->entries, &memoryMap->key, &entrySize, &version);

        if (status == kSLStatusBufferTooSmall)
        {
            SLFree(memoryMap->entries);
            continue;
        }

        if (SLStatusIsError(status)) goto fail;
        return memoryMap;
    }

fail:
    if (memoryMap->entries) SLFree(memoryMap->entries);
    SLFree(memoryMap);

    return kOSNullPointer;
}

SLMemoryMap *SLBootServicesTerminate(void)
{
    SLBootServicesCheck(kOSNullPointer);

    SLMemoryMap *finalMemoryMap = SLBootServicesGetMemoryMap();
    if (!finalMemoryMap) return kOSNullPointer;

    SLStatus status = SLBootServicesGetCurrent()->terminate(SLGetMainImageHandle(), finalMemoryMap->key);

    if (SLStatusIsError(status)) {
        SLFree(finalMemoryMap->entries);
        SLFree(finalMemoryMap);

        return kOSNullPointer;
    } else {
        gSLBootServicesEnabled = false;

        return finalMemoryMap;
    }
}
