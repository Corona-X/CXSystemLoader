/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLMemoryAllocator.h - Bootloader Memory Allocation Routines     */
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

    OSPrivate OSSize SLGetObjectSize(OSAddress object);
    OSPrivate bool SLDoesOwnMemory(OSAddress object);
    OSPrivate OSAddress SLAllocate(OSSize size);
    OSPrivate OSAddress SLReallocate(OSAddress object, OSSize newSize);
    OSPrivate void SLFree(OSAddress object);

    OSExport SLMemoryPool gSLPoolInfo;
    OSExport SLHeap gSLCurrentHeap;

    #if kCXBuildDev
        OSExport OSCount gSLMemoryAllocationCount;
        OSExport OSCount gSLMemoryFreeCount;
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLMEMORYALLOCATOR__) */
