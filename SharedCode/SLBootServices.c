#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/C/CLMemory.h>

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

OSAddress SLBootServicesAllocateLowPages(OSCount pages)
{
    SLBootServicesCheck(kOSNullPointer);
    OSAddress result = 0xFFFF;

    SLStatus status = SLBootServicesGetCurrent()->allocatePages(kSLAllocTypeMaxAddress, kSLMemoryTypeLoaderData, pages, &result);
    if (SLStatusError(status)) return kOSNullPointer;

    return result;
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

    CLMemorySetValue(memoryMap, sizeof(SLMemoryMap), 0);
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
        memoryMap->entryCount = (memoryMapSize / entrySize);
        memoryMap->entries = SLAllocate(memoryMapSize);
        if (!memoryMap->entries) goto fail;

        //memoryMapSize = SLGetObjectSize(memoryMap->entries);
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

OSAddress *SLBootServicesLocateHandles(SLProtocol protocol, OSCount *count)
{
    SLBootServicesCheck(kOSNullPointer);

    SLBootServices *bootServices = SLBootServicesGetCurrent();
    OSAddress *devices = kOSNullPointer;
    OSCount handleCount;

    SLStatus status = bootServices->localeHandles(kSLSearchTypeByProtocol, &protocol, kOSNullPointer, &handleCount, &devices);
    if (SLStatusIsError(status)) return kOSNullPointer;

    OSAddress *results = SLAllocate(handleCount * sizeof(OSAddress));
    if (!results) goto fail;

    for (OSCount i = 0; i < handleCount; i++)
    {
        status = bootServices->handleProtocol(devices[i], &protocol, &results[i]);
        if (SLStatusIsError(status)) goto fail;
    }

    if (!SLBootServicesFree(devices)) goto fail;
    if (count) (*count) = handleCount;
    return results;

fail:
    if (results) SLFree(results);
    SLBootServicesFree(devices);

    return kOSNullPointer;
}

OSAddress SLBootServicesLocateProtocol(SLProtocol protocol)
{
    SLBootServicesCheck(kOSNullPointer);
    OSAddress result;

    SLStatus status = SLBootServicesGetCurrent()->locateProtocol(&protocol, kOSNullPointer, &result);
    if (status == kSLStatusNotFound) return kOSNullPointer;
    if (SLStatusIsError(status)) return kOSNullPointer;

    return result;
}

OSNoReturn void SLBootServicesExit(SLStatus status)
{
    SLBootServicesGetCurrent()->exit(SLGetMainImageHandle(), status, 0, kOSNullPointer);

    // If we're still here, there was an error somewhere...
    SLRuntimeServicesResetSystem(kSLResetTypeShutdown, status, kOSNullPointer);
}
