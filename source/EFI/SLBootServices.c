#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/XKDebugLog.h>
#include <Kernel/XKUtilities.h>

typedef struct __SLBootServicesTerminateHandler {
    void (*function)(SLMemoryMap *finalMap, OSAddress context);
    OSAddress context;
    struct __SLBootServicesTerminateHandler *next;
} SLBootServicesTerminateHandler;

SLBootServicesTerminateHandler *gSLBootServicesFirstHandler = kOSNullPointer;

SLBootServices *SLBootServicesGetCurrent(void)
{
    return SLSystemTableGetCurrent()->bootServices;
}

bool SLBootServicesHaveTerminated(void)
{
    return !gSLBootServicesEnabled;
}

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
    SLStatus status = SLBootServicesGetCurrent()->terminate(SLGetMainImageHandle(), finalMemoryMap->key);

    if (SLStatusIsError(status)) {
        XKPrintString(" [Failed]\n");
        SLFree(finalMemoryMap);

        return kOSNullPointer;
    } else {
        XKPrintString(" [Success]\n");
        gSLBootServicesEnabled = false;

        return finalMemoryMap;
    }
}
