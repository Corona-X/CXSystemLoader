#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLFormattedPrint.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/XKMemory.h>

SLSystemTable *SLSystemTableGetCurrent(void)
{
    return gSLLoaderSystemTable;
}

CPRootDescriptor *SLSystemTableGetACPIRoot(SLSystemTable *table)
{
    CPRootDescriptor *root = kOSNullPointer;

    for (OSIndex i = 0; i < table->numberOfConfigTables; i++)
    {
        SLConfigTable *configTable = &table->configTables[i];
        OSUIDIntelData acpiTableID = kSLACPITableID;

        if (!XKMemoryCompare(&configTable->id, &acpiTableID, sizeof(OSUIDIntelData)))
            root = configTable->pointer;
    }

    if (root && CPRootDescriptorValidate(root)) {
        return root;
    } else {
        return kOSNullPointer;
    }
}

#if kCXBuildDev
    void SLSystemTableDumpConfigTables(SLSystemTable *table)
    {
        UIntN count = table->numberOfConfigTables;

        if (!count)
        {
            SLPrintString("Warning: No UEFI Config Tables Found!\n");
            return;
        }

        SLPrintString("Dumping %d Configuration Tables:\n", count);
        SLPrintString("Note: First table is at %p\n", table->configTables);

        for (OSIndex i = 0; i < count; i++)
        {
            SLConfigTable *configTable = &table->configTables[i];
            OSUTF8Char *id = SLUIDToString(&configTable->id);

            SLPrintString("Config Table %d: {\n", i);
            SLPrintString("    ID: %s\n", id);
            SLPrintString("    Address: %p\n", configTable->pointer);
            SLPrintString("} ");

            SLFree(id);
        }

        SLPrintString("\n\n");
    }
#endif /* kCXBuildDev */
