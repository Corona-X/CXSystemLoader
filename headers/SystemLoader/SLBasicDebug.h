/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBasicDebug.h - Debug routines for SystemLoader                */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBASICDEBUG__
#define __SYSTEMLOADER_SLBASICDEBUG__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode && kCXBootloaderCode && kCXBuildDev

//OSPrivate void SLShowDelay(const char *s, UInt64 seconds);
//OSPrivate bool SLPromptUser(const char *s);

//OSPrivate void SLDumpBasicState(XKProcessorBasicState *state);
//OSPrivate void SLDumpSystemState(XKProcessorSystemState *state);
//OSPrivate void SLDumpDebugState(XKProcessorDebugState *state);
//OSPrivate void SLDumpProcessorState(bool standard, bool system, bool debug);

OSPrivate void SLMemoryAllocatorDumpMainPool(void);
OSPrivate void SLMemoryAllocatorDumpHeapInfo(void);

//OSPrivate bool SLGraphicsOutputDumpInfo(void);
//OSPrivate void SLSystemTableDumpConfigTables(SLSystemTable *table);

#endif /* kCXAssemblyCode && kCXBootloaderCode && kCXBuildDev */

#endif /* !defined(__SYSTEMLOADER_SLBASICDEBUG__) */
