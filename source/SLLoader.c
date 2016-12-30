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

OSNoReturn void SLShutdownVirtualMachine(void)
{
    // Let the user read on-screen output first
    if (!SLBootServicesHaveTerminated())
        SLWaitForKeyPress();

    // QEMU lets us just do this
    XKWriteIOByte(0xF4, 0x00);
    OSEndCode();
}

SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable)
{
    SLPrintError("%s(%p, %p) called\n", __func__, imageHandle, systemTable);
    SLPrintError("Note: Should be %p, %p", SLSystemTableGetCurrent(), SLGetMainImageHandle());

    #if kCXBuildDev
        //SLSetVideoColor(0x8202FF, false);
        SLPrintString(kSLLoaderWelcomeString);
        //SLSetVideoColor(0x00FFFFFF, false);

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
            SLShutdownVirtualMachine();
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
            SLShutdownVirtualMachine();
        }

        SLPrintString("Note: Discovered valid ACPI Root Table at %p\n", rootTable);
        SLPrintString("Note: Root Table has %zu entries\n", CPRootTableGetEntryCount(rootTable));
    #endif /* kCXBuildDev */

    SLShutdownVirtualMachine();
}
