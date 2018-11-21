#include <SystemLoader/EFI/SLSystemTable.h>
#include <Kernel/C/XKMemory.h>

OSAddress SLSystemTableLocateConfigTable(SLProtocol tableID)
{
    SLSystemTable *systemTable = SLSystemTableGetCurrent();

    for (OSIndex i = 0; ((OSCount)i) < systemTable->numberOfConfigTables; i++)
    {
        SLConfigTable *table = &systemTable->configTables[i];

        if (!XKMemoryCompare(&table->id, &tableID, sizeof(SLProtocol)))
            return table->pointer;
    }

    return kOSNullPointer;
}
