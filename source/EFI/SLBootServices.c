#include <SystemLoader/SystemLoader.h>

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
    OSAddress result = base;
    SLStatus status = gSLLoaderSystemTable->bootServices->allocatePages(kSLAllocTypeAtAddress, kSLMemoryTypeLoaderData, pages, &result);
    if (SLStatusIsError(status)) return false;
    if (result != base) return false;
    return base;
}

OSBuffer SLBootServicesAllocateAnyPages(OSCount pages)
{
    OSAddress result;
    SLStatus status = gSLLoaderSystemTable->bootServices->allocatePages(kSLAllocTypeAnyPages, kSLMemoryTypeLoaderData, pages, &result);
    if (SLStatusIsError(status)) return kOSBufferEmpty;
    
    return OSBufferMake(result, (pages * kSLBootPageSize));
}

OSBuffer SLBootServicesAllocate(OSSize size)
{
    OSAddress result;
    SLStatus status = gSLLoaderSystemTable->bootServices->allocate(kSLMemoryTypeLoaderData, size, &result);
    if (SLStatusIsError(status)) return kOSBufferEmpty;
    return OSBufferMake(result, size);
}

bool SLBootServicesFree(OSBuffer buffer)
{
    SLStatus status = gSLLoaderSystemTable->bootServices->free(buffer.address);
    return SLStatusIsError(status);
}

CXKMemoryMap *SLBootServicesGetMemoryMap(void)
{
    CXKMemoryMap *map = SLAllocate(sizeof(CXKMemoryMap)).address;
    OSSize neededSize;
    UIntN entrySize;
    UInt32 version;
    
    SLStatus status = gSLLoaderSystemTable->bootServices->getMemoryMap(&neededSize, map->entries, &map->key, &entrySize, &version);
    if (entrySize != sizeof(CXKMemoryMapEntry)) status = kSLStatusWrongSize;
    if (status == kSLStatusBufferTooSmall) status = kSLStatusSuccess;
    if (version != 1) status = kSLStatusIncompatibleVersion;
    
    if (SLStatusIsError(status))
    {
        SLFree(map);
        
        return kOSNullPointer;
    }
    
again:
    map->entryCount = (neededSize / sizeof(CXKMemoryMapEntry));
    OSBuffer entryBuffer = SLAllocate(neededSize);
    map->entries = entryBuffer.address;
    
    neededSize = entryBuffer.size;
    status = gSLLoaderSystemTable->bootServices->getMemoryMap(&neededSize, map->entries, &map->key, &entrySize, &version);
    
    if (status == kSLStatusBufferTooSmall)
    {
        SLFree(map->entries);
        
        goto again;
    }
    
    if (entrySize != sizeof(CXKMemoryMapEntry)) status = kSLStatusWrongSize;
    if (version != 1) status = kSLStatusIncompatibleVersion;
    
    if (SLStatusIsError(status))
    {
        SLFree(map->entries);
        SLFree(map);
        
        return kOSNullPointer;
    }
    
    return map;
}

CXKMemoryMap *SLBootServicesTerminate(void)
{
    CXKMemoryMap *finalMemoryMap = SLBootServicesGetMemoryMap();
    if (!finalMemoryMap) return kOSNullPointer;
    
    SLStatus status = gSLLoaderSystemTable->bootServices->terminate(gSLLoaderImageHandle, finalMemoryMap->key);
    
    if (SLStatusIsError(status)) {
        SLFree(finalMemoryMap);
        
        return kOSNullPointer;
    } else {
        gSLBootServicesEnabled = false;
        
        return finalMemoryMap;
    }
}

bool SLDelayProcessor(UIntN time, bool useBootServices)
{
    if (useBootServices && gSLBootServicesEnabled) {
        SLStatus status = SLBootServicesGetCurrent()->stall(time);
        return !SLStatusIsError(status);
    } else {
        // Try to mimic BS stall() function
        for (volatile UInt64 i = 0; i < (time * 100); i++);
        return true;
    }
}

char SLWaitForKeyPress(void)
{
    SLStatus status = gSLLoaderSystemTable->stdin->reset(gSLLoaderSystemTable->stdin, false);
    if (SLStatusIsError(status)) return 0;
    status = kSLStatusNotReady;
    SLKeyPress key;
    
    while (status == kSLStatusNotReady)
        status = gSLLoaderSystemTable->stdin->readKey(gSLLoaderSystemTable->stdin, &key);
    
    return key.keycode;
}

bool SLBootServicesOutputString(OSUTF16Char *string)
{
    SLStatus status = gSLLoaderSystemTable->stdout->printUTF16(gSLLoaderSystemTable->stdout, string);
    return !SLStatusIsError(status);
}
