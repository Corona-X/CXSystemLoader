/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLMemoryAllocator.h - Bootloader Memory Allocation Routines     */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 30.10.2016 - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLMEMORYALLOCATOR__
#define __SYSTEMLOADER_SLMEMORYALLOCATOR__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#define kSLMemoryAllocatorDefaultPoolSize    (1 << 23)
#define kSLMemoryAllocatorPoolAlignment      16
#define kSLMemoryAllocatorAllocAlignment     16

#if !kCXAssemblyCode

typedef struct __SLMemoryNode {
    struct __SLMemoryNode *next;
    UIntN size;
} SLMemoryNode;

typedef struct {
    OSAddress address;
    OSSize size;
    OSSize usedSize;
    SLMemoryNode *head;

    // We keep these in during any build,
    // but they're only used in dev/debug builds.
    // In release, we need to keep a comsistent strcture size,
    // as this structure is used across binaries.
    OSCount allocCount;
    OSCount freeCount;
} SLMemoryPool;

typedef struct {
    OSAddress baseAddress;
    OSSize currentSize;
    OSSize maxSize;

    bool initialized;
    bool shouldFree;
} SLHeap;

#if kCXBootloaderCode
    OSPrivate OSAddress SLMemoryAllocatorInit(void);
    OSPrivate void SLMemoryAllocatorSetHeap(OSAddress heap, OSSize newSize);
    OSPrivate OSSize SLMemoryAllocatorGetCurrentHeapSize(void);
    OSPrivate OSAddress SLMemoryAllocatorGetHeapAddress(void);
    OSPrivate OSSize SLMemoryAllocatorGetHeapSize(void);
    OSPrivate void SLMemoryAllocatorOnTerminateBoot(void);

    OSPrivate OSSize SLGetObjectSize(OSAddress object);
    OSPrivate bool SLDoesOwnMemory(OSAddress object);
    OSPrivate OSAddress SLAllocate(OSSize size);
    OSPrivate OSAddress SLReallocate(OSAddress object, OSSize newSize);
    OSPrivate void SLFree(OSAddress object);

    #if kCXBuildDev
        OSPrivate OSCount SLMemoryAllocatorGetPoolAllocCount(void);
        OSPrivate OSCount SLMemoryAllocatorGetPoolFreeCount(void);

        OSPrivate void SLMemoryAllocatorDumpMainPool(void);
        OSPrivate void SLMemoryAllocatorDumpHeapInfo(void);
    #endif /* kCXBuildDev */

    // These should be copied to CXKernelLoader
    OSExport SLMemoryPool gSLMainPoolInfo;
    OSExport SLHeap gSLCurrentHeap;
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLMEMORYALLOCATOR__) */
