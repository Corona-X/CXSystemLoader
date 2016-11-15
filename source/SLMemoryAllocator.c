#include <SystemLoader/SystemLoader.h>
#include <System/OSByteMacros.h>

static bool initialized = false;
static SLMemoryPool poolInfo;
static SLHeap heap;

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

        heap.shouldFree = true;
    }

    OSBuffer heapBuffer = SLMemoryAllocatorGetHeap();

    if (!OSBufferIsEmpty(heapBuffer))
        initialized = true;

    return heapBuffer;
}

void SLMemoryAllocatorSetHeap(OSBuffer newHeap)
{
    if (heap.shouldFree)
        SLBootServicesFree(SLMemoryAllocatorGetHeap());

    if (!OSBufferIsEmpty(newHeap))
    {
        heap.baseAddress = OSAlignUpward((UIntN)newHeap.address, kSLMemoryAllocatorPoolAlignment);
        heap.maxSize = newHeap.size;
        heap.currentSize = 0;
    }
}

OSSize SLMemoryAllocatorGetCurrentHeapSize(void)
{
    return heap.currentSize;
}

OSBuffer SLMemoryAllocatorGetHeap(void)
{
    return OSBufferMake(heap.baseAddress, heap.maxSize);
}

OSAddress SLExpandHeap(OSCount moreBytes)
{
    OSSize newSize = heap.currentSize + moreBytes;

    if (OSExpect(newSize < heap.maxSize))
    {
        OSAddress returnAddress = heap.baseAddress + heap.currentSize;
        heap.currentSize = newSize;
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
                SLPrintError("Error: Memory List is Corrupted!\n");
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
            SLPrintError("Error: Memory List is Corrupted!\n");
            SLPrintString("Tried to free {%zu from %p}\n", object.size, object.address);
            SLPrintString("Pool is {%zu from %p}\n", pool->size, pool->address);
            SLPrintString("Error found on node {%zu from %p}\n", node->size, node);
            SLPrintString("Heap is at %p and is %zu bytes\n", heap.baseAddress, heap.currentSize);
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

    return ((buffer.address >= poolInfo.address) && ((buffer.address + buffer.size) <= (poolInfo.address + poolInfo.size)));
}

OSBuffer SLAllocate(OSSize size)
{
    if (!size) return kOSBufferEmpty;

    OSSize allocSize = size + sizeof(OSSize);
    allocSize = OSAlignUpward(allocSize, kSLMemoryAllocatorAllocAlignment);
    allocSize = OSRoundToPowerOf2(allocSize);
    OSAddress result;

    while (!(result = SLAllocateInPool(&poolInfo, allocSize)))
    {
        OSCount extendCount = OSAlignUpward(allocSize, kSLBootPageSize);
        OSBuffer buffer = OSBufferMake(SLExpandHeap(extendCount), extendCount);

        if (!buffer.address)
        {
            SLPrintError("Error: Out of Memory!!\n");

            return kOSBufferEmpty;
        }

        SLExpandPool(&poolInfo, extendCount, buffer.address);
        SLFreeInPool(&poolInfo, buffer);
    }

    (*((OSSize *)result)) = allocSize;
    result += sizeof(OSSize);

    return OSBufferMake(result, allocSize - sizeof(OSSize));
}

OSBuffer SLReallocate(OSAddress object, OSSize newSize)
{
    if (!SLDoesOwnMemory(object))
    {
        SLPrintError("Error: SLReallocate() on object not inside pool!\n");
        SLUnrecoverableError();
    }

    OSSize *size = object; size--;
    OSBuffer buffer = OSBufferMake(size, *size);

    OSBuffer newBuffer = SLAllocate(newSize);
    if (OSBufferIsEmpty(newBuffer)) return kOSBufferEmpty;

    CXKMemoryCopy(object, newBuffer.address, buffer.size - sizeof(OSSize));
    SLFreeInPool(&poolInfo, buffer);

    return newBuffer;
}

void SLFree(OSAddress object)
{
    if (!object) return;

    OSSize *size = object; size--;
    OSBuffer buffer = OSBufferMake(size, *size);

    if (!SLDoesOwnMemory(object))
    {
        SLPrintError("Error: SLFree() on object not inside pool!\n");
        SLPrintString("Pool: {%zu from %p}; Object: {%zu from %p}\n", poolInfo.size, poolInfo.address, buffer.size, buffer.address);
        SLUnrecoverableError();
    }

    SLFreeInPool(&poolInfo, buffer);
}

#if kCXBuildDev
    SLMemoryPool *SLMemoryAllocatorGetMainPool(void)
    {
        return &poolInfo;
    }

    SLHeap *SLMemoryAllocatorGetRealHeap(void)
    {
        return &heap;
    }
#endif /* kCXBuildDev */
