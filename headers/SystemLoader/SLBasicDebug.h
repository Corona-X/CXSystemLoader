/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBasicDebug.h - Debug routines for SystemLoader                */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBASICDEBUG__
#define __SYSTEMLOADER_SLBASICDEBUG__ 1

#include <Corona-X.h>
#include <System/OSTypes.h>
#include <System/OSCompilerMacros.h>

//OSPrivate void SLShowDelay(const char *s, UInt64 seconds);
//OSPrivate bool SLPromptUser(const char *s);

//OSPrivate void SLDumpBasicState(XKProcessorBasicState *state);
//OSPrivate void SLDumpSystemState(XKProcessorSystemState *state);
//OSPrivate void SLDumpDebugState(XKProcessorDebugState *state);
//OSPrivate void SLDumpProcessorState(bool standard, bool system, bool debug);

//OSPrivate void __XKBitmapFontInitialize(void);
//OSPrivate void __XKInputConsoleInitAllEFI(void);
//OSPrivate void __XKSerialConsoleInitAll(void);
//OSPrivate void __XKVideoConsoleInitAll(void);
//OSPrivate void __SLLibraryInitialize(void);

//OSPrivate void SLDumpConsoles(void);

OSPrivate void SLMemoryAllocatorDumpMainPool(void);
OSPrivate void SLMemoryAllocatorDumpHeapInfo(void);

//OSPrivate bool SLGraphicsOutputDumpInfo(void);

//OSPrivate void SLSystemTableDumpConfigTables(SLSystemTable *table);

#endif /* !defined(__SYSTEMLOADER_SLBASICDEBUG__) */
