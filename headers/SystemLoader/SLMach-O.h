/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLMach-O.h - Mach-O loader for bootloader/kernel loader         */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.8.2016   -  3:00 PM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLMACHO__
#define __SYSTEMLOADER_SLMACHO__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <System/Executables/OSMach-O.h>

// 32 KB - 1 page stack by default
// The lowest page of the stack will be marked as not present.
// This is to caught possible stack overflows.
#define kSLMachODefaultStackSize (1 << 15)

#if !kCXAssemblyCode

typedef struct {
    OSAddress base;
    OSSize size;

    OSMOHeader *header;

    OSIndex dataSectionIndex;
    OSOffset symbolTableOffset;
    OSCount symbolCount;
    OSOffset stringsOffset;
    OSSize stringsSize;

    OSAddress stackAddress;
    OSSize stackSize;

    UInt64 minLoadAddress;
    UInt64 maxLoadAddress;

    OSAddress loadAddress;
    OSSize loadedSize;

    OSMOThreadStateNative *entryPoint;
} SLMachOFile;

#if kCXBootloaderCode
    OSPrivate SLMachOFile *SLMachOFileOpenMapped(OSAddress base, OSSize size);
    OSPrivate OSInteger SLMachOSetSymbolValues(SLMachOFile *file, const OSUTF8Char *const *symbols, OSCount count, const OSAddress *const *values, OSSize *symbolSizes);
    OSPrivate OSAddress SLMachOGetSymbolAddress(SLMachOFile *file, const OSUTF8Char *symbol);
    OSPrivate bool SLMachOCallVoidFunction(SLMachOFile *file, const OSUTF8Char *symbolName);
    OSPrivate OSNoReturn void SLMachOExecute(SLMachOFile *file);
    OSPrivate void SLMachOFileClose(SLMachOFile *file);

    // Note: These functions are used to map a file into memory properly.
    // They should be defined in another object file linked with SLMach-O.c
    OSPrivate OSAddress SLAllocatePages(OSCount pages);
    OSPrivate void SLFreePages(OSAddress base, OSCount pages);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLMACHO__) */
