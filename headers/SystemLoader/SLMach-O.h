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

OSPrivate SLMachOFile *SLMachOFileOpenMapped(OSAddress base, OSSize size);
OSPrivate OSInteger SLMachOSetSymbolValues(SLMachOFile *file, const OSUTF8Char *const *symbols, OSCount count, const OSAddress *const *values, OSSize *symbolSizes);
OSPrivate OSNoReturn void SLMachOExecute(SLMachOFile *file);
OSPrivate void SLMachOFileClose(SLMachOFile *file);

#endif /* !defined(__SYSTEMLOADER_SLMACHO__) */
