#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/XKShared.h>

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
            XKLog(kXKLogLevelVerbose, "Warning: No UEFI Config Tables Found!\n");
            return;
        }

        XKLog(kXKLogLevelVerbose, "Dumping %d UEFI Configuration Tables:\n", count);
        XKLog(kXKLogLevelVerbose, "First table is at %p\n", table->configTables);

        for (OSIndex i = 0; i < count; i++)
        {
            SLConfigTable *configTable = &table->configTables[i];
            OSUTF8Char *id = XKUIDToString(&configTable->id);

            XKPrintString("Config Table %d: {\n", i);
            XKLog(kXKLogLevelVerbose, "    ID: %s\n", id);
            XKLog(kXKLogLevelVerbose, "    Address: %p\n", configTable->pointer);
            XKLog(kXKLogLevelVerbose, "} ");

            SLFree(id);
        }

        XKPrintString("\n\n");
    }
#endif /* kCXBuildDev */
