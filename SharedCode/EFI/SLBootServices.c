#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/C/SLMemory.h>

SLBootServices *SLBootServicesGetCurrent(void)
{
    return SLSystemTableGetCurrent()->bootServices;
}

bool SLBootServicesHaveTerminated(void)
{
    return !gSLBootServicesEnabled;
}

bool SLBootServicesAllocatePages(OSAddress base, OSCount pages)
{
    SLBootServicesCheck(false);

    OSAddress result = base;
    SLStatus status = SLBootServicesGetCurrent()->allocatePages(kSLAllocTypeAtAddress, kSLMemoryTypeLoaderData, pages, &result);
    if (SLStatusIsError(status)) return false;
    if (result != base) return false;
    return base;
}

OSBuffer SLBootServicesAllocateAnyPages(OSCount pages)
{
    SLBootServicesCheck(kOSBufferEmpty);

    OSAddress result;
    SLStatus status = SLBootServicesGetCurrent()->allocatePages(kSLAllocTypeAnyPages, kSLMemoryTypeLoaderData, pages, &result);
    if (SLStatusIsError(status)) return kOSBufferEmpty;

    return OSBufferMake(result, (pages * kSLBootPageSize));
}

bool SLBootServicesFree(OSAddress address)
{
    SLBootServicesCheck(false);

    SLStatus status = SLBootServicesGetCurrent()->free(address);
    return !SLStatusIsError(status);
}

SLMemoryMap *SLBootServicesGetMemoryMap(void)
{
    SLBootServicesCheck(kOSNullPointer);

    SLMemoryMap *map = SLAllocate(sizeof(SLMemoryMap)).address;
    XKMemorySetValue(map, sizeof(SLMemoryMap), 0);
    OSSize neededSize = kOSNullPointer;
    UIntN entrySize = kOSNullPointer;
    UInt32 version = 0;

    SLStatus status = SLBootServicesGetCurrent()->getMemoryMap(&neededSize, map->entries, &map->key, &entrySize, &version);
    if (entrySize != sizeof(SLMemoryDescriptor)) status = kSLStatusWrongSize;
    if (status == kSLStatusBufferTooSmall) status = kSLStatusSuccess;
    if (status == kSLStatusBadArgument) status = kSLStatusSuccess;
    if (version != 1) status = kSLStatusIncompatibleVersion;
    if (SLStatusIsError(status)) goto fail;

    for (OSCount i = 0; i < 3; i++)
    {
        map->entryCount = (neededSize / sizeof(SLMemoryDescriptor));
        OSBuffer entryBuffer = SLAllocate(neededSize);
        map->entries = entryBuffer.address;

        neededSize = entryBuffer.size;
        status = SLBootServicesGetCurrent()->getMemoryMap(&neededSize, map->entries, &map->key, &entrySize, &version);
        if (entrySize != sizeof(SLMemoryDescriptor)) status = kSLStatusWrongSize;
        if (version != 1) status = kSLStatusIncompatibleVersion;
        if (status == kSLStatusBufferTooSmall) continue;
        if (SLStatusIsError(status)) goto fail;

        return map;
    }

fail:
    if (map->entries) SLFree(map->entries);
    SLFree(map);

    return kOSNullPointer;
}

SLMemoryMap *SLBootServicesTerminate(void)
{
    SLBootServicesCheck(kOSNullPointer);

    SLMemoryMap *finalMemoryMap = SLBootServicesGetMemoryMap();
    if (!finalMemoryMap) return kOSNullPointer;

    SLStatus status = SLBootServicesGetCurrent()->terminate(SLGetMainImageHandle(), finalMemoryMap->key);

    if (SLStatusIsError(status)) {
        SLFree(finalMemoryMap);

        return kOSNullPointer;
    } else {
        gSLBootServicesEnabled = false;

        return finalMemoryMap;
    }
}
