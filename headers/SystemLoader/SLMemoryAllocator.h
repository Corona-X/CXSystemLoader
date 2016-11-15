/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLMemoryAllocator.h - Bootloader Memory Allocation Routines     */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 30.10.2016 - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 30.10.2016 - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLMEMORYALLOCATOR__
#define __SYSTEMLOADER_SLMEMORYALLOCATOR__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#define kSLMemoryAllocatorDefaultPoolSize    (1 << 26)
#define kSLMemoryAllocatorPoolAlignment      16
#define kSLMemoryAllocatorAllocAlignment     16

typedef struct __SLMemoryNode {
    struct __SLMemoryNode *next;
    UIntN size;
} SLMemoryNode;

typedef struct {
    OSAddress address;
    OSSize size;
    OSSize usedSize;
    SLMemoryNode *head;
} SLMemoryPool;

typedef struct {
    OSAddress baseAddress;
    OSSize currentSize;
    OSSize maxSize;

    bool shouldFree;
} SLHeap;

#if kCXBootloaderCode
    OSPrivate OSBuffer SLMemoryAllocatorInit(void);
    OSPrivate void SLMemoryAllocatorSetHeap(OSBuffer heap);
    OSPrivate OSSize SLMemoryAllocatorGetCurrentHeapSize(void);
    OSPrivate OSBuffer SLMemoryAllocatorGetHeap(void);

    //OSPrivate OSBuffer SLAllocateAligned(OSSize size, OSOffset alignment);
    OSPrivate bool SLDoesOwnMemory(OSAddress object);
    OSPrivate OSBuffer SLAllocate(OSSize size);
    OSPrivate OSBuffer SLReallocate(OSAddress object, OSSize newSize);
    OSPrivate void SLFree(OSAddress object);

    #if kCXBuildDev
        OSPrivate SLMemoryPool *SLMemoryAllocatorGetMainPool(void);
        OSPrivate SLHeap *SLMemoryAllocatorGetRealHeap(void);
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLMEMORYALLOCATOR__) */
