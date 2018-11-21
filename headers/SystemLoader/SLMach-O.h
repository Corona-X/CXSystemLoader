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

// 8 MB stack by default
#define kSLMachODefaultStackSize (1 << 23)

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

    OSAddress loadAddress;
    OSSize loadedSize;

    OSMOThreadStateNative *entryPoint;
} SLMachOFile;

#if kCXBootloaderCode
    OSPrivate SLMachOFile *SLMachOFileOpenMapped(OSAddress base, OSSize size);
    OSPrivate OSInteger SLMachOSetSymbolValues(SLMachOFile *file, const OSUTF8Char *const *symbols, OSCount count, const OSAddress *const *values, OSSize *symbolSizes);
    OSPrivate bool SLMachOCallVoidFunction(SLMachOFile *file, const OSUTF8Char *name);
    OSPrivate OSNoReturn void SLMachOExecute(SLMachOFile *file);
    OSPrivate void SLMachOFileClose(SLMachOFile *file);

    // Note: This function is used to map a file into memory properly. It should be defined in another object file linked with SLMach-O.c
    OSPrivate OSAddress SLAllocateAnyPages(OSCount pages);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLMACHO__) */
