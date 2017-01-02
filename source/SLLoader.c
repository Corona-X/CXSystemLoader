#include <SystemLoader/SystemLoader.h>
#include <System/OSCompilerMacros.h>
#include <Kernel/XKShared.h>

void SLDumpConsoles(void)
{
    XKPrintString("Consoles:\n");

    SLConsole *console = gSLFirstConsole;
    
    while (console)
    {
        XKPrintString("%u: %p\n", console->id, console);
        XKPrintString("  --> context: %p\n", console->context);
        XKPrintString("  --> output:  %p\n", console->output);
        XKPrintString("  --> next:    %p\n", console->next);
        
        console = console->next;
    }
}

void SLPromptTests(void)
{
    bool runRequested = SLPromptUser("Run Tests");
    
    if (runRequested)
    {
        SLShowDelay("Running", 2);
        SLRunTests();
    }
}

bool isProbablyVM = false;

OSNoReturn void SLShutdownMachine(void)
{
    if (isProbablyVM) {
        // QEMU lets us just do this
        XKWriteIOByte(0xF4, 0x00);
        OSEndCode();
    } else {
        // This only gets called before BootServices exit for now...
        SLLeave(kSLStatusLoadError);
    }
}

SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable)
{
    #if kCXBuildDev
        SLPrintString(kSLLoaderWelcomeString);

        SLGraphicsOutputDumpInfo();
        SLDumpConsoles();

        SLPromptTests();

        SLPrintString("Starting Loader... Initial CPU States:\n");
        SLDumpProcessorState(true, true, true);

        SLPrintString("Dumping UEFI System Table Configuration Entries:\n");
        SLSystemTableDumpConfigTables(SLSystemTableGetCurrent());

        CPRootDescriptor *rootDescriptor = SLSystemTableGetACPIRoot(SLSystemTableGetCurrent());

        if (!rootDescriptor)
        {
            SLPrintString("Error: No valid ACPI Root Table found! (Does thie machine have the proper version of ACPI?)\n");
            SLWaitForKeyPress();
            SLShutdownMachine();
        }

        UInt8 manufacturer[7];
        XKMemoryCopy(CPRootDescriptorGetManufacturerID(rootDescriptor), manufacturer, 6);
        manufacturer[6] = 0;

        SLPrintString("Note: Discovered valid ACPI Root Descriptor at %p\n", rootDescriptor);
        SLPrintString("Note: ACPI manufacturer is '%s'\n", manufacturer);

        CPRootTable *rootTable = CPRootDescriptorGetRootTable(rootDescriptor);

        if (!rootTable)
        {
            SLPrintString("Error: Could not detect valid ACPI Root Table from the Root Descriptor!\n");
            SLWaitForKeyPress();
            SLShutdownMachine();
        }

        SLPrintString("Note: Discovered valid ACPI Root Table at %p\n", rootTable);
        SLPrintString("Note: Root Table has %zu entries\n", CPRootTableGetEntryCount(rootTable));
    #endif /* kCXBuildDev */

    SLWaitForKeyPress();
    SLShutdownMachine();
}
