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

typedef struct {
    OSAddress base;
    OSSize size;

    OSMOHeader *header;
    OSCount sectionCount;
    OSOffset symbolOffset;
    OSCount symbolCount;
    OSOffset stringOffset;

    OSAddress activateBase;
    OSSize virtualSize;

    OSMOThreadStateNative *entryPoint;
} SLMachOFile;

OSPrivate SLMachOFile *SLMachOProcess(OSAddress base, OSSize size);
OSPrivate void SLMachOClose(SLMachOFile *file);

OSPrivate bool SLMachOValidate(SLMachOFile *file);
OSPrivate OSInteger SLMachOReplaceSymbols(SLMachOFile *file, const OSUTF8Char *const *symbols, OSCount count, const OSAddress *const *values, OSSize *symbolSizes);
OSPrivate void SLMachOExecute(SLMachOFile *file);

#endif /* !defined(__SYSTEMLOADER_SLMACHO__) */
