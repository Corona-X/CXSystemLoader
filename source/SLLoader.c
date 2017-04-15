#include <SystemLoader/SystemLoader.h>
#include <System/OSCompilerMacros.h>
#include <Kernel/XKShared.h>

void SLPromptTests(void)
{
    bool runRequested = SLPromptUser("Run Tests");

    if (runRequested)
    {
        SLShowDelay("Running", 2);
        SLRunTests();
    }
}

void SLDebugMain()
{
    SLGraphicsOutputDumpInfo();
    SLDumpConsoles();

    SLSystemTableDumpConfigTables(SLSystemTableGetCurrent());
    SLPromptTests();
}

SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable)
{
    XKLog(kXKLogLevelInfo, kSLLoaderWelcomeString);
    SLDebugMain();

    XKLog(kXKLogLevelInfo, "Starting Loader...\n");
    XKLog(kXKLogLevelVerbose, "Initial Processor State:\n");
    SLDumpProcessorState(true, true, true);

    bool haveCorePower = CPInitializeForSystemLoader();
    if (!haveCorePower) return kSLStatusLoadError;

    //

    XKConsoleWaitForInput();
    CPShutdownMachine();
}
