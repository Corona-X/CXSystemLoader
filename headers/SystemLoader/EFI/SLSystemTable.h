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
    SLSimpleTextInput   *stdin;
    OSAddress           stdoutHandle;
    SLSimpleTextOutput  *stdout;
    OSAddress           stderrHandle;
    SLSimpleTextOutput  *stderr;
    SLRuntimeServices   *runtimeServices;
    SLBootServices      *bootServices;
    UInt64              numberOfConfigTables;
    SLConfigTable       *configTables;
} SLSystemTable;

#if kCXBootloaderCode
    OSPrivate SLSystemTable *SLSystemTableGetCurrent(void);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLSYSTEMTABLE__) */
