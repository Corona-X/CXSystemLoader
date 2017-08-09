#include <SystemLoader/SystemLoader.h>
#include <Kernel/C/XKMemory.h>

// TODO: SLRuntimeServices.h/c
// TODO: SLSystemLoader.h
// TODO: Mach-O Loader
// TODO: SLConfigFile.c
// TODO: Test on a real machine!

SLStatus CXSystemLoaderMain(OSUnused OSAddress imageHandle, OSUnused SLSystemTable *systemTable)
{
    // Hmm.... how to start....
    SLPrintString(kSLLoaderWelcomeString);

    OSCount count;
    SLBlockIO **blockDevices = SLBlockIOGetAll(&count);

    if (!count) {
        SLPrintString("No block devices found.\n");
        SLBootConsoleReadKey(true);

        return kSLStatusLoadError;
    } else {
        bool *hasSystem = SLAllocate(count * sizeof(bool));
        SLPrintString("Found %u block devices.\n", count);
        SLPrintString("Detecting systems...\n");
        OSCount systemCount = 0;

        for (OSIndex i = 0; (UInt64)i < count; i++)
        {
            UInt64 systemVersion = SLIsSystemPartition(blockDevices[i]);

            if (systemVersion) {
                hasSystem[i] = true;
                systemCount++;

                SLPrintString("%u: ", systemCount);
                SLPrintSystemVersionInfo(&systemVersion);
                SLPrintString("\n");
            } else {
                hasSystem[i] = false;
            }
        }

        SLBlockIO *selected = kOSNullPointer;
        OSOffset selectedOffset = 0;
        OSIndex selectedIndex;

        if (!systemCount) {
            SLPrintString("No systems found.\n");
            SLBootConsoleReadKey(true);

            return kSLStatusNotFound;
        } else if (systemCount == 1) {
            // Wow this is easy...
            selectedIndex = 0;
        } else {
            // TODO: Get the user to pick a system
            selectedIndex = 0;
        }

        for (OSIndex i = 0; (UInt64)i < count; i++)
        {
            if (hasSystem[i] && (selectedIndex == selectedOffset++))
            {
                selected = blockDevices[i];
                break;
            }
        }

        SLFree(blockDevices);
        SLFree(hasSystem);

        // Either I screwed up or the user screwed up...
        // I don't really care, just get out
        if (!selected)
        {
            SLPrintString("Error: Could not find the selected system!\n");
            return kSLStatusNotFound;
        }

        // Yay we can load a system now!!!
        SLLoadSystemOrLeave(selected);
    }
}
