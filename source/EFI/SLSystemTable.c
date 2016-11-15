#include <SystemLoader/SystemLoader.h>

SLSystemTable *SLSystemTableGetCurrent(void)
{
    return gSLLoaderSystemTable;
}

CDACPITableRoot *SLSystemTableGetACPIRoot(SLSystemTable *table)
{
    return kOSNullPointer;
}

#if kCXBuildDev
    void SLSystemTableDumpConfigTables(SLSystemTable *table)
    {
        SLPrintString("Dumping %d System Tables:", table->numberOfConfigTables);

        for (UIntN i = 0; i < table->numberOfConfigTables; i++)
        {
            SLConfigTable *config = &table->configTables;
            OSUTF8Char *id = SLUIDToString(&config->id);

            SLPrintString("Config Table %d: {\n", i);
            SLPrintString("    ID: %s\n", id);
            SLPrintString("    Address: %p\n", config->pointer);
            SLPrintString("} ");

            SLFree(id);
        }

        SLPrintString("\n\n");
    }
#endif /* kCXBuildDev */
