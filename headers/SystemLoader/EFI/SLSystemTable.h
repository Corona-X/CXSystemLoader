/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLSystemTable.h - EFI System Table Declaration                  */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLSYSTEMTABLE__
#define __SYSTEMLOADER_EFI_SLSYSTEMTABLE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLRuntimeServices.h>
#include <SystemLoader/EFI/SLBootConsole.h>

#if !kCXAssemblyCode

typedef struct {
    SLProtocol  id;
    OSAddress   pointer;
} SLConfigTable;

typedef struct {
    SLTableHeader       header;
    OSUTF16Char         *firmwareVendor;
    UInt32              firmwareRevision;
    OSAddress           stdinHandle;
    SLConsoleInput      *stdin;
    OSAddress           stdoutHandle;
    SLConsoleOutput     *stdout;
    OSAddress           stderrHandle;
    SLConsoleOutput     *stderr;
    SLRuntimeServices   *runtimeServices;
    SLBootServices      *bootServices;
    UInt64              numberOfConfigTables;
    SLConfigTable       *configTables;
} SLSystemTable;

#if kCXBootloaderCode || kCXKernelCode
    // Note: This is implemented in SLRuntimeServices.c
    OSPrivate SLSystemTable *SLSystemTableGetCurrent(void);

    OSPrivate OSAddress SLSystemTableLocateConfigTable(SLProtocol tableID);

    #if kCXBuildDev
        OSPrivate void SLSystemTableDumpConfigTables(SLSystemTable *table);
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode || kCXKernelCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLSYSTEMTABLE__) */
