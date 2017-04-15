#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLLibrary.h>
#include <System/OSByteMacros.h>
#include <Kernel/XKShared.h>

#if kCXBuildDev
    static OSCount gSLMemoryAllocationCount = 0;
    static OSCount gSLMemoryFreeCount = 0;
#endif

static bool initialized = false;
static SLMemoryPool gSLPoolInfo;
static SLHeap gSLCurrentHeap;

OSPrivate OSAddress SLAllocateInPool(SLMemoryPool *pool, OSSize size);
OSPrivate void SLExpandPool(SLMemoryPool *pool, OSCount moreBytes, OSAddress base);
OSPrivate void SLFreeInPool(SLMemoryPool *pool, OSBuffer object);
OSPrivate OSAddress SLExpandHeap(OSCount moreBytes);

OSBuffer SLMemoryAllocatorInit(void)
{
    if (initialized) return SLMemoryAllocatorGetHeap();

    OSBuffer newHeap = SLBootServicesAllocateAnyPages(kSLMemoryAllocatorDefaultPoolSize / kSLBootPageSize);

    if (!OSBufferIsEmpty(newHeap))
    {
        SLMemoryAllocatorSetHeap(newHeap);

        gSLCurrentHeap.shouldFree = true;
    }

    OSBuffer heapBuffer = SLMemoryAllocatorGetHeap();

    if (!OSBufferIsEmpty(heapBuffer))
        initialized = true;

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
                XKLog(kXKLogLevelError, "Memory List is Corrupted!\n");
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
            XKLog(kXKLogLevelError, "Memory List is Corrupted!\n");
            XKLog(kXKLogLevelVerbose, "Tried to free {%zu from %p}\n", object.size, object.address);
            XKLog(kXKLogLevelVerbose, "On node {%zu from %p}\n", node->size, node);
            SLMemoryAllocatorDumpHeapInfo();
            SLMemoryAllocatorDumpMainPool();
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
            XKLog(kXKLogLevelError, "Error: Out of Memory!!\n");

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
        XKLog(kXKLogLevelError, "Error: SLReallocate() on object not inside pool!\n");
        SLUnrecoverableError();
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
        XKLog(kXKLogLevelError, "SLFree() on object not inside pool!\n");
        XKLog(kXKLogLevelVerbose, "Object: {%zu from %p}\n", gSLPoolInfo.size, gSLPoolInfo.address, buffer.size, buffer.address);
        SLMemoryAllocatorDumpHeapInfo();
        SLMemoryAllocatorDumpMainPool();
        SLUnrecoverableError();
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

    void SLMemoryAllocatorDumpHeapInfo(void)
    {
        XKLog(kXKLogLevelVerbose, "Heap Info:\n");
        XKLog(kXKLogLevelVerbose, "Base Address: %p\n", gSLCurrentHeap.baseAddress);
        XKLog(kXKLogLevelVerbose, "Size: %u/%u ", gSLCurrentHeap.currentSize, gSLCurrentHeap.maxSize);
        XKLog(kXKLogLevelVerbose, "(0x%zX/0x%zX)\n", gSLCurrentHeap.currentSize, gSLCurrentHeap.maxSize);
        XKLog(kXKLogLevelVerbose, "Will Free:    %s\n", (gSLCurrentHeap.shouldFree ? "yes" : "no"));
    }

    void SLMemoryAllocatorDumpMainPool(void)
    {
        XKLog(kXKLogLevelVerbose, "Pool Info:\n");
        XKLog(kXKLogLevelVerbose, "Base Address: %p\n", gSLPoolInfo.address);
        XKLog(kXKLogLevelVerbose, "Size: %u/%u ", gSLPoolInfo.usedSize, gSLPoolInfo.size);
        XKLog(kXKLogLevelVerbose, "(0x%zX/0x%zX)\n", gSLPoolInfo.usedSize, gSLPoolInfo.size);
        XKLog(kXKLogLevelVerbose, "Node List:\n");

        SLMemoryNode *node = gSLPoolInfo.head;
        OSCount spaces = 0;

        while (node)
        {
            XKLog(kXKLogLevelVerbose, "");

            for (OSCount i = 0; i < spaces; i++)
                XKPrintString(" ");

            XKPrintString("--> %p (0x%zX bytes) --> %p\n", node, node->size, node->next);
            node = node->next;
            spaces += 2;
        }
    }
#endif /* kCXBuildDev */
