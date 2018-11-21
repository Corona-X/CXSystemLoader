#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLLibrary.h>
#include <System/OSByteMacros.h>
#include <Kernel/C/XKMemory.h>

OSPrivate void SLExpandPool(SLMemoryPool *pool, OSCount moreBytes, OSAddress base);
OSPrivate OSAddress SLExpandHeap(OSCount moreBytes);

OSPrivate OSAddress SLAllocateInPool(SLMemoryPool *pool, OSSize size);
OSPrivate void SLFreeInPool(SLMemoryPool *pool, OSAddress object, OSSize objectSize);

// These 2 symbols will be overrides in CXKernelLoader
__attribute__((section("__DATA,__data"))) SLMemoryPool gSLMainPoolInfo = {
    .address = kOSNullPointer,
    .size = 0,
    .usedSize = 0,
    .head = kOSNullPointer,

    .allocCount = 0,
    .freeCount = 0
};

__attribute__((section("__DATA,__data"))) SLHeap gSLCurrentHeap = {
    .baseAddress = kOSNullPointer,
    .currentSize = 0,
    .maxSize = 0,
    .initialized = false,
    .shouldFree = false,
};

#pragma mark - Allocator Functions

OSAddress SLMemoryAllocatorInit(void)
{
    if (gSLCurrentHeap.initialized)
        return SLMemoryAllocatorGetHeapAddress();

    OSSize newHeapSize = kSLMemoryAllocatorDefaultPoolSize >> kSLBootPageShift;
    OSAddress newHeap = SLBootServicesAllocateAnyPages(newHeapSize);

    if (newHeap) {
        SLMemoryAllocatorSetHeap(newHeap, newHeapSize << kSLBootPageShift);
        gSLCurrentHeap.shouldFree = true;
    } else {
        if (kCXBuildDev) {
            SLPrintString("Error: Heap buffer allocation failed! (%s in %s)\n", __func__, __FILE__);
            SLPrintString("This Error is unrecoverable.\n");
        } else {
            SLPrintString("Error: Heap allocation failed! [%d]\n", __LINE__);
        }

        SLUnrecoverableError();
    }

    OSAddress heap = SLMemoryAllocatorGetHeapAddress();
    if (heap) gSLCurrentHeap.initialized = true;

    return heap;
}

#pragma mark - Heap Functions

void SLMemoryAllocatorSetHeap(OSAddress newHeap, OSSize newSize)
{
    if (gSLCurrentHeap.shouldFree)
        SLBootServicesFreePages(gSLCurrentHeap.baseAddress, gSLCurrentHeap.currentSize >> kSLBootPageShift);

    if (newHeap)
    {
        gSLCurrentHeap.baseAddress = OSAlignUpward((UInt64)newHeap, kSLMemoryAllocatorPoolAlignment);
        gSLCurrentHeap.maxSize = newSize;
        gSLCurrentHeap.currentSize = 0;
    }
}

OSSize SLMemoryAllocatorGetCurrentHeapSize(void)
{
    return gSLCurrentHeap.currentSize;
}

OSAddress SLMemoryAllocatorGetHeapAddress(void)
{
    return gSLCurrentHeap.baseAddress;
}

OSSize SLMemoryAllocatorGetHeapSize(void)
{
    return gSLCurrentHeap.maxSize;
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

#pragma mark - Pool Functions

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

    // We don't have space for that size...
    return kOSNullPointer;
}

void SLFreeInPool(SLMemoryPool *pool, OSAddress object, OSSize objectSize)
{
    pool->usedSize -= objectSize;

    SLMemoryNode **ref = &pool->head;
    SLMemoryNode *node;

    while ((node = (*ref)))
    {
        if (object <= (OSAddress)node)
        {
            SLMemoryNode *end = object + objectSize;
            SLMemoryNode *freeNode = object;

            if (end > node)
            {
                // Something bad happened...
                SLPrintString("Memory List is Corrupted! [%d]\n", __LINE__);
                SLUnrecoverableError();
            }

            if (end == node) {
                freeNode->size = objectSize + node->size;
                freeNode->next = node->next;
            } else {
                freeNode->size = objectSize;
                freeNode->next = node;
            }

            (*ref) = node = freeNode;

            if (ref != &pool->head)
            {
                SLMemoryNode *refNode = (SLMemoryNode *)ref;

                if ((((UInt8 *)ref) + refNode->size) == object)
                {
                    refNode->size += node->size;
                    refNode->next = node->next;
                }
            }

            return;
        }

        if (object < (OSAddress)(((UInt8 *)node) + node->size))
        {
            // Something bad happened again...
            SLPrintString("Memory List is Corrupted! [%d]\n", __LINE__);

            #if kCXBuildDev
                SLPrintString("Note: Tried to free {%zu from %p} on node {%zu from %p}\n",
                              objectSize, object, node->size, node);

                SLMemoryAllocatorDumpHeapInfo();
                SLMemoryAllocatorDumpMainPool();
            #endif /* kCXBuildDev */

            SLUnrecoverableError();
        }

        ref = &node->next;
    }

    if ((ref == &pool->head) || ((((UInt8 *)ref) + ((SLMemoryNode *)ref)->size)) != object) {
        SLMemoryNode *objectNode = (SLMemoryNode *)object;
        objectNode->next = kOSNullPointer;
        objectNode->size = objectSize;

        (*ref) = objectNode;
    } else {
        SLMemoryNode *refNode = (SLMemoryNode *)ref;
        refNode->size += objectSize;
    }
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

#pragma mark - Base Functions

OSSize SLGetObjectSize(OSAddress object)
{
    OSSize *sizePointer = object - sizeof(OSSize);
    OSSize size = *sizePointer;

    return size;
}

bool SLDoesOwnMemory(OSAddress object)
{
    OSSize size = SLGetObjectSize(object);

    return ((object >= gSLMainPoolInfo.address) && ((object + size) <= (gSLMainPoolInfo.address + gSLMainPoolInfo.size)));
}

OSAddress SLAllocate(OSSize size)
{
    if (!size) return kOSNullPointer;

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

    while (!(result = SLAllocateInPool(&gSLMainPoolInfo, allocSize)))
    {
        OSCount extendCount = OSAlignUpward(allocSize, kSLBootPageSize);
        OSAddress heapBase = SLExpandHeap(extendCount);

        if (!heapBase)
        {
            SLPrintString("Out of Memory!\n");
            return kOSNullPointer;
        }

        SLExpandPool(&gSLMainPoolInfo, extendCount, heapBase);
        SLFreeInPool(&gSLMainPoolInfo, heapBase, extendCount);
    }

    (*((OSSize *)result)) = allocSize;
    result += sizeof(OSSize);

    if (kCXBuildDev)
        gSLMainPoolInfo.allocCount++;

    return result;
}

OSAddress SLReallocate(OSAddress object, OSSize newSize)
{
    if (!SLDoesOwnMemory(object))
    {
        SLPrintString("SLReallocate() on object not inside pool!\n");

        if (kCXBuildDev)
            SLPrintString("Object: %p, Requested Size: %zu\n", object, newSize);

        return kOSNullPointer;
    }

    OSSize *sizePointer = object - sizeof(OSSize);
    OSSize size = *sizePointer;

    OSAddress newObject = SLAllocate(newSize);
    if (!newObject) return kOSNullPointer;

    XKMemoryCopy(object, newObject, size - sizeof(OSSize));
    SLFreeInPool(&gSLMainPoolInfo, sizePointer, size);

    return newObject;
}

void SLFree(OSAddress object)
{
    if (!object) return;

    OSSize *sizePointer = object - sizeof(OSSize);
    OSSize size = *sizePointer;

    if (!SLDoesOwnMemory(object))
    {
        SLPrintString("SLFree() on object not inside pool! (Has the current pool changed since this object was allocated?)\n");

        #if kCXBuildDev
            SLPrintString("Object: {%u bytes @ %p}\n", size, sizePointer);

            SLMemoryAllocatorDumpHeapInfo();
            SLMemoryAllocatorDumpMainPool();
        #endif /* kCXBuildDev */

        return;
    }

    SLFreeInPool(&gSLMainPoolInfo, sizePointer, size);

    if (kCXBuildDev)
        gSLMainPoolInfo.freeCount++;
}

#pragma mark - Debug Functions

#if kCXBuildDev

OSCount SLMemoryAllocatorGetPoolAllocCount(void)
{
    return gSLMainPoolInfo.allocCount;
}

OSCount SLMemoryAllocatorGetPoolFreeCount(void)
{
    return gSLMainPoolInfo.freeCount;
}

void SLMemoryAllocatorDumpHeapInfo(void)
{
    SLPrintString("Heap Info:\n");
    SLPrintString("Base Address: %p\n", gSLCurrentHeap.baseAddress);
    SLPrintString("Size: %u/%u ",       gSLCurrentHeap.currentSize, gSLCurrentHeap.maxSize);
    SLPrintString("(0x%zX/0x%zX)\n",    gSLCurrentHeap.currentSize, gSLCurrentHeap.maxSize);
    SLPrintString("Will Free:    %s\n", (gSLCurrentHeap.shouldFree ? "yes" : "no"));
}

void SLMemoryAllocatorDumpMainPool(void)
{
    SLPrintString("Pool Info:\n");
    SLPrintString("Base Address: %p\n", gSLMainPoolInfo.address);
    SLPrintString("Size: %u/%u ",       gSLMainPoolInfo.usedSize, gSLMainPoolInfo.size);
    SLPrintString("Allocations: %u\n",  gSLMainPoolInfo.allocCount);
    SLPrintString("Frees: %u\n",        gSLMainPoolInfo.freeCount);
    SLPrintString("(0x%zX/0x%zX)\n",    gSLMainPoolInfo.usedSize, gSLMainPoolInfo.size);
    SLPrintString("Node List:\n");

    SLMemoryNode *node = gSLMainPoolInfo.head;
    OSCount spaces = 0;

    while (node)
    {
        SLPrintString("");

        for (OSCount i = 0; i < spaces; i++)
            SLPrintString(" ");

        SLPrintString("--> %p (0x%zX bytes) --> %p\n", node, node->size, node->next);
        node = node->next;
        spaces += 2;
    }
}

#endif /* kCXBuildDev */
