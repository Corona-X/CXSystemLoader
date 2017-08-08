/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLDebug.h - Debug routines for SystemLoader                     */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLDEBUG__
#define __SYSTEMLOADER_SLDEBUG__ 1

#include <Corona-X.h>
#include <System/OSTypes.h>
#include <Kernel/Shared/XKProcessorState.h>
#include <SystemLoader/EFI/SLSystemTable.h>

OSPrivate void SLEarlyPrint(const char *format, ...);

OSPrivate void SLShowDelay(const char *s, UInt64 seconds);
OSPrivate bool SLPromptUser(const char *s);

OSPrivate void SLDumpBasicState(XKProcessorBasicState *state);
OSPrivate void SLDumpSystemState(XKProcessorSystemState *state);
OSPrivate void SLDumpDebugState(XKProcessorDebugState *state);
OSPrivate void SLDumpProcessorState(bool standard, bool system, bool debug);

OSPrivate void __XKBitmapFontInitialize(void);
OSPrivate void __XKInputConsoleInitAllEFI(void);
OSPrivate void __XKSerialConsoleInitAll(void);
OSPrivate void __XKVideoConsoleInitAll(void);
OSPrivate void __SLLibraryInitialize(void);

OSPrivate void SLDumpConsoles(void);

OSPrivate void SLMemoryAllocatorDumpMainPool(void);
OSPrivate void SLMemoryAllocatorDumpHeapInfo(void);

OSPrivate bool SLGraphicsOutputDumpInfo(void);

OSPrivate void SLSystemTableDumpConfigTables(SLSystemTable *table);

#endif /* !defined(__SYSTEMLOADER_SLDEBUG__) */
