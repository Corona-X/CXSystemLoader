/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLSystemTable.h - EFI System Table Declaration                  */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLSYSTEMTABLE__
#define __SYSTEMLOADER_SLSYSTEMTABLE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLRuntimeServices.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <Kernel/CoreDriver/CDACPI.h>

typedef struct {
    SLProtocol id;
    OSAddress pointer;
} SLConfigTable;

typedef struct {
    SLTableHeader header;
    UInt16 *firmwareVendor;
    UInt32 firmwareRevision;
    UInt32 padding;
    OSAddress unused1;
    SLSimpleTextInput *stdin;
    OSAddress unused2;
    SLSimpleTextOutput *stdout;
    OSAddress unused3[2];
    SLRuntimeServices *runtimeServices;
    SLBootServices *bootServices;
    UIntN numberOfConfigTables;
    SLConfigTable configTables[];
} SLSystemTable;

#if kCXBootloaderCode
    OSPrivate SLSystemTable *SLSystemTableGetCurrent(void);
    OSPrivate CDACPITableRoot *SLSystemTableGetACPIRoot(SLSystemTable *table);

    #if kCXBuildDev
        OSPrivate void SLSystemTableDumpConfigTables(SLSystemTable *table);
    #endif /* kCXBuildDev */
#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLSYSTEMTABLE__) */
