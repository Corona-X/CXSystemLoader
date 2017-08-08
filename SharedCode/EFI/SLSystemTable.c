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
