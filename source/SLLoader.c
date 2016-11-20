#include <SystemLoader/SystemLoader.h>

SLStatus CXSystemLoaderMain(OSAddress imageHandle, SLSystemTable *systemTable)
{
    #if kCXBuildDev
        SLSetVideoColor(0x8202FF, false);
        SLPrintString(kSLLoaderWelcomeString);
        SLSetVideoColor(0x00FFFFFF, false);

        SLPS("Consoles:\n");

        SLConsole *console = gSLFirstConsole;

        while (console)
        {
            SLPS("%u: %p\n", console->id, console);
            SLPS("  --> context: %p\n", console->context);
            SLPS("  --> output:  %p\n", console->output);
            SLPS("  --> next:    %p\n", console->next);

            console = console->next;
        }

        bool runRequested = SLPromptUser("Run Tests", 0x03F8);

        if (runRequested)
        {
            SLShowDelay("Running", 2);
            SLRunTests();
        }

        SLPrintString("Starting Loader. CPU States:\n");
        SLDumpProcessorState(true, true, true);

        if (!SLBootServicesHaveTerminated())
            SLWaitForKeyPress();
    #endif /* kCXBuildDev */

    if (SLBootServicesHaveTerminated()) {
        for ( ; ; )
            SLDelayProcessor(10000000, false);
    } else {
        return kSLStatusSuccess;
    }
}
