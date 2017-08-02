#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLLibrary.h>
#include <System/OSByteMacros.h>
#include <Kernel/XKShared.h>

#if kCXBuildDev
    #include <SystemLoader/SLDebug.h>

    static OSCount gSLMemoryAllocationCount = 0;
    static OSCount gSLMemoryFreeCount = 0;
#endif

static bool gSLAllocatorInitialized = false;

static SLMemoryPool gSLPoolInfo = {
    .address = kOSNullPointer,
    .size = 0,
    .usedSize = 0,
    .head = kOSNullPointer
};

static SLHeap gSLCurrentHeap = {
    .baseAddress = kOSNullPointer,
    .currentSize = 0,
    .maxSize = 0,
    .shouldFree = kOSNullPointer
};

OSPrivate void SLMemoryAllocatorOnBootServicesTerminate(SLMemoryMap *finalMemoryMap, OSAddress context);
OSPrivate OSAddress SLAllocateInPool(SLMemoryPool *pool, OSSize size);
OSPrivate void SLExpandPool(SLMemoryPool *pool, OSCount moreBytes, OSAddress base);
OSPrivate void SLFreeInPool(SLMemoryPool *pool, OSBuffer object);
OSPrivate OSAddress SLExpandHeap(OSCount moreBytes);

void SLMemoryAllocatorOnBootServicesTerminate(SLMemoryMap *finalMemoryMap, OSAddress context)
{
    gSLCurrentHeap.shouldFree = false;
}

OSBuffer SLMemoryAllocatorInit(void)
{
    if (gSLAllocatorInitialized) return SLMemoryAllocatorGetHeap();

    OSBuffer newHeap = SLBootServicesAllocateAnyPages(kSLMemoryAllocatorDefaultPoolSize / kSLBootPageSize);

    if (!OSBufferIsEmpty(newHeap)) {
        SLMemoryAllocatorSetHeap(newHeap);

        gSLCurrentHeap.shouldFree = true;
    } else {
        #if kCXBuildDev
            SLEarlyPrint("Error: Heap buffer allocation failed! (%s in %s)\n", __func__, __FILE__);
            SLEarlyPrint("This Error is unrecoverable.\n");
        #endif /* kCXBuildDev */

        SLUnrecoverableError();
    }

    OSBuffer heapBuffer = SLMemoryAllocatorGetHeap();

    if (!OSBufferIsEmpty(heapBuffer))
    {
        gSLAllocatorInitialized = true;

        SLBootServicesRegisterTerminationFunction(SLMemoryAllocatorOnBootServicesTerminate, kOSNullPointer);
    }

    return heapBuffer;
}

void SLMemoryAllocatorSetHeap(OSBuffer newHeap)
{
    if (gSLCurrentHeap.shouldFree)
        SLBootServicesFree(SLMemoryAllocatorGetHeap().address);

    if (!OSBufferIsEmpty(newHeap))
    {
        gSLCurrentHeap.baseAddress = OSAlignUpward((UIntN)newHeap.address, kSLMemoryAllocatorPoolAlignment);
        gSLCurrentHeap.maxSize = newHeap.size;
        gSLCurrentHeap.currentSize = 0;
    }
}

OSSize SLMemoryAllocatorGetCurrentHeapSize(void)
{
    return gSLCurrentHeap.currentSize;
}

OSBuffer SLMemoryAllocatorGetHeap(void)
{
    return OSBufferMake(gSLCurrentHeap.baseAddress, gSLCurrentHeap.maxSize);
}

OSAddress SLExpandHeap(OSCount moreBytes)
{
    OSSize newSize = gSLCurrentHeap.currentSize + moreBytes;

    if (OSExpect(newSize < gSLCurrentHeap.maxSize))
    {
        OSAddress returnAddress = gSLCurrentHeap.baseAddress + gSLCurrentHeap.currentSize;
        gSLCurrentHeap.currentSize = newSize;
        return returnAddress;
    }

    return kOSNullPointer;
}

OSAddress SLAllocateInPool(SLMemoryPool *pool, OSSize size)
{
    OSSize bytesLeft = (pool->size - pool->usedSize);

    if (size <= bytesLeft)
    {
        SLMemoryNode **ref = &pool->head;
        SLMemoryNode *node;

        while ((node = (*ref)))
        {
            if (node->size < size)
            {
                ref = &node->next;
                continue;
            }

            SLMemoryNode *result = node;

            if (node->size == size) {
                (*ref) = node->next;
            } else {
                node = (((UInt8 *)node) + size);
                node->next = result->next;
                node->size = result->size - size;
                (*ref) = node;
            }

            pool->usedSize += size;
            return result;
        }
    }

    return kOSNullPointer;
}

void SLExpandPool(SLMemoryPool *pool, OSCount moreBytes, OSAddress base)
{
    if (!pool->size) {
        pool->usedSize = pool->size = moreBytes;
        pool->address = base;
    } else {
        pool->usedSize += moreBytes;
        pool->size += moreBytes;
    }
}

void SLFreeInPool(SLMemoryPool *pool, OSBuffer object)
{
    pool->usedSize -= object.size;

    SLMemoryNode **ref = &pool->head;
    SLMemoryNode *node;

    while ((node = (*ref)))
    {
        if (object.address <= (OSAddress)node)
        {
            SLMemoryNode *end = object.address + object.size;
            SLMemoryNode *freeNode = object.address;

            if (end > node)
            {
                // Something bad happened...
                XKLog(kXKLogLevelError, kXKLogSubsystemLoader "Memory List is Corrupted! [0x%d]", __LINE__);
                SLUnrecoverableError();
            }

            if (end == node) {
                freeNode->size = object.size + node->size;
                freeNode->next = node->next;
            } else {
                freeNode->size = object.size;
                freeNode->next = node;
            }

            (*ref) = node = freeNode;

            if (ref != &pool->head)
            {
                SLMemoryNode *refNode = (SLMemoryNode *)ref;

                if ((((UInt8 *)ref) + refNode->size) == object.address)
                {
                    refNode->size += node->size;
                    refNode->next = node->next;
                }
            }

            return;
        }

        if (object.address < (OSAddress)(((UInt8 *)node) + node->size))
        {
            // Something bad happened again...
            XKLog(kXKLogLevelError, kXKLogSubsystemLoader "Memory List is Corrupted! [0x%d]", __LINE__);

            #if kCXBuildDev
                XKLog(kXKLogLevelVerbose, kXKLogSubsystemLoader
                      "Note: Tried to free {%zu from %p} on node {%zu from %p}",
                      object.size, object.address, node->size, node);

                SLMemoryAllocatorDumpHeapInfo();
                SLMemoryAllocatorDumpMainPool();
            #endif /* kCXBuildDev */

            SLUnrecoverableError();
        }

        ref = &node->next;
    }

    if ((ref == &pool->head) || ((((UInt8 *)ref) + ((SLMemoryNode *)ref)->size)) != object.address) {
        SLMemoryNode *objectNode = (SLMemoryNode *)object.address;
        objectNode->next = kOSNullPointer;
        objectNode->size = object.size;

        (*ref) = objectNode;
    } else {
        SLMemoryNode *refNode = (SLMemoryNode *)ref;
        refNode->size += object.size;
    }
}

bool SLDoesOwnMemory(OSAddress object)
{
    OSSize *size = object; size--;
    OSBuffer buffer = OSBufferMake(size, *size);

    return ((buffer.address >= gSLPoolInfo.address) && ((buffer.address + buffer.size) <= (gSLPoolInfo.address + gSLPoolInfo.size)));
}

OSBuffer SLAllocate(OSSize size)
{
    if (!size) return kOSBufferEmpty;

    OSSize allocSize = size + sizeof(OSSize);
    allocSize = OSAlignUpward(allocSize, kSLMemoryAllocatorAllocAlignment);

    // Allocate next power of 2 or,
    // if the block is larger than the current page size,
    // allocate the next multiple of the page size
    if (allocSize < kSLBootPageSize) {
        allocSize = OSRoundToPowerOf2(allocSize);
    } else {
        allocSize = OSAlignUpward(allocSize, kSLBootPageSize);
    }

    OSAddress result;

    while (!(result = SLAllocateInPool(&gSLPoolInfo, allocSize)))
    {
        OSCount extendCount = OSAlignUpward(allocSize, kSLBootPageSize);
        OSBuffer buffer = OSBufferMake(SLExpandHeap(extendCount), extendCount);

        if (!buffer.address)
        {
            XKLog(kXKLogLevelWarning, kXKLogSubsystemLoader "Out of Memory!");

            return kOSBufferEmpty;
        }

        SLExpandPool(&gSLPoolInfo, extendCount, buffer.address);
        SLFreeInPool(&gSLPoolInfo, buffer);
    }

    (*((OSSize *)result)) = allocSize;
    result += sizeof(OSSize);

    #if kCXBuildDev
        gSLMemoryAllocationCount++;
    #endif

    return OSBufferMake(result, allocSize - sizeof(OSSize));
}

OSBuffer SLReallocate(OSAddress object, OSSize newSize)
{
    if (!SLDoesOwnMemory(object))
    {
        XKLog(kXKLogLevelError, kXKLogSubsystemLoader "SLReallocate() on object not inside pool!");

        #if kCXBuildDev
            XKLog(kXKLogLevelVerbose, kXKLogSubsystemLoader "Object: %p, Requested Size: %zu", object, newSize);
        #endif /* kCXBuildDev */

        return kOSBufferEmpty;
    }

    OSSize *size = object; size--;
    OSBuffer buffer = OSBufferMake(size, *size);

    OSBuffer newBuffer = SLAllocate(newSize);
    if (OSBufferIsEmpty(newBuffer)) return kOSBufferEmpty;

    XKMemoryCopy(object, newBuffer.address, buffer.size - sizeof(OSSize));
    SLFreeInPool(&gSLPoolInfo, buffer);

    return newBuffer;
}

void SLFreeBuffer(OSBuffer buffer)
{
    SLFree(buffer.address);
}

void SLFree(OSAddress object)
{
    if (!object) return;

    OSSize *size = object; size--;
    OSBuffer buffer = OSBufferMake(size, *size);

    if (!SLDoesOwnMemory(object))
    {
        XKLog(kXKLogLevelError, kXKLogSubsystemLoader "SLFree() on object not inside pool!"
              "(Has the current pool changed since this object was allocated?)");

        #if kCXBuildDev
            OSUTF8Char *string = XKBufferToString(&buffer);

            XKLog(kXKLogLevelVerbose, kXKLogSubsystemLoader "Object: %s\n", string);
            SLMemoryAllocatorDumpHeapInfo();
            SLMemoryAllocatorDumpMainPool();

            SLFree(string);
        #endif /* kCXBuildDev */

        return;
    }

    SLFreeInPool(&gSLPoolInfo, buffer);

    #if kCXBuildDev
        gSLMemoryFreeCount++;
    #endif
}

#if kCXBuildDev
    SLMemoryPool *SLMemoryAllocatorGetMainPool(void)
    {
        return &gSLPoolInfo;
    }

    SLHeap *SLMemoryAllocatorGetHeapInfo(void)
    {
        return &gSLCurrentHeap;
    }

    OSCount SLMemoryAllocatorGetAllocCount(void)
    {
        return gSLMemoryAllocationCount;
    }

    OSCount SLMemoryAllocatorGetFreeCount(void)
    {
        return gSLMemoryFreeCount;
    }
#endif /* kCXBuildDev */
