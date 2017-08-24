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

OSPrivate bool SLMachOValidate(OSAddress base, OSSize size);
OSPrivate bool SLMachOReplaceSymbols(OSAddress imageBase, OSSize imageSize, const OSUTF8Char **symbols, OSCount count, const OSAddress *values, OSSize *symbolSizes);
OSPrivate void SLMachOExecute(OSAddress base, OSSize size);

#endif /* !defined(__SYSTEMLOADER_SLMACHO__) */
