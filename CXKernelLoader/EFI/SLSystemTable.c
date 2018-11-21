#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLBasicIO.h>
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

#if kCXBuildDev

void SLSystemTableDumpConfigTables(SLSystemTable *table)
{
    UIntN count = table->numberOfConfigTables;

    if (!count)
    {
        SLPrintString("No UEFI Config Table Found.\n");
        return;
    }

    SLPrintString("Found %u UEFI Config Tables (from %p):\n", count, table->configTables);

    for (OSIndex i = 0; i < (OSIndex)count; i++)
    {
        SLConfigTable *configTable = &table->configTables[i];
        OSUIDIntelData *uid = &configTable->id;

        SLPrintString("%02u: %08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X at %p\n", i, uid->group0, uid->group1, uid->group2,
            uid->group3[0], uid->group3[1], uid->group3[2], uid->group3[3],
            uid->group3[4], uid->group3[5], uid->group3[6], uid->group3[7],
        configTable->pointer);
    }
}

#endif /* kCXBuildDev */
