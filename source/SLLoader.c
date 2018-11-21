#include <SystemLoader/SystemLoader.h>
#include <Kernel/C/XKMemory.h>

// TODO: SLRuntimeServices.h/c
// TODO: Test on a real machine!

static SLBlockIO *SLSelectSystemDevice(SLBlockIO **blockDevices, OSCount count)
{
    bool *hasSystem = SLAllocate(count * sizeof(bool));
    SLPrintString("Detecting installed systems...\n");
    OSCount systemCount = 0;

    if (!hasSystem)
    {
        SLPrintString("Error: Could not allocate memory.\n");
        return kOSNullPointer;
    }

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

    bool autoboot = SLConfigGetBool(SLConfigGetCurrent(), kSLConfigKeyAutoboot, true);
    SLBlockIO *selected = kOSNullPointer;
    OSOffset selectedOffset = 0;
    OSIndex selectedIndex;

    if (!systemCount) {
        SLPrintString("No systems found.\n");
        SLFree(hasSystem);

        SLPrintString("Press any key to exit.\n");
        SLBootConsoleReadKey(true);

        SLLeave(kSLStatusSuccess);
    } else if (systemCount == 1 && autoboot) {
        // Wow this is easy...
        selectedIndex = 0;

        SLPrintString("System 1 selected.\n");
    } else if (systemCount == 1 && !autoboot) {
        SLPrintString("System 1 selected. Press any key to confirm.\n");
        SLBootConsoleReadKey(true);

        selectedIndex = 0;
    } else {
        SLPrintString("Selection: ");
        SLPrintBufferFlush();

        bool success;
        selectedIndex = SLBootConsoleReadNumber(10, &success);

        if (!success)
        {
            SLPrintString("Invalid Response. System #1 selected...\n");
            selected = 0;

            if (!autoboot)
            {
                SLPrintString("Press any key to confirm.\n");
                SLBootConsoleReadKey(true);
            }
        }
    }

    for (OSIndex i = 0; (UInt64)i < count; i++)
    {
        if (hasSystem[i] && (selectedIndex == selectedOffset++))
        {
            selected = blockDevices[i];
            break;
        }
    }

    if (kCXBuildDev)
        SLPrintString("Selected system is on device %u\n", selectedOffset);

    SLFree(hasSystem);
    return selected;
}

#pragma mark - CXSystemLoader Main

SLStatus CXSystemLoaderMain(OSUnused OSAddress imageHandle, OSUnused SLSystemTable *systemTable)
{
    // Hmm.... how to start....
    SLPrintString(kSLLoaderWelcomeString);
    SLConfigGetCurrent(); // Load config

    #if kCXBuildDev
        SLConfigDump(SLConfigGetCurrent());
    #endif /* kCXBuildDev */

    OSCount count;
    SLBlockIO **blockDevices = SLBlockIOGetAll(&count);

    if (!count) {
        SLPrintString("No block devices found.\n");
        SLBootConsoleReadKey(true);

        return kSLStatusLoadError;
    } else {
        if (kCXBuildDev)
            SLPrintString("Found %u block devices.\n", count);

        SLBlockIO *selected = SLSelectSystemDevice(blockDevices, count);
        SLFree(blockDevices);

        // Either I screwed up or the user screwed up...
        // I don't really care, just get out
        if (!selected)
        {
            SLPrintString("Error: Could not find the selected system!\n");
            SLBootConsoleReadKey(true);

            return kSLStatusNotFound;
        }

        // Yay we can load a system now!!!
        SLLoadSystemOrLeave(selected);
    }
}
