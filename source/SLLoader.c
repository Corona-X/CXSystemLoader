#include <SystemLoader/SystemLoader.h>
#include <SystemLoader/EFI/SLBlockIO.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <System/Archives/OSCAR.h>
#include <Kernel/C/XKMemory.h>

#include <System/Executables/OSELF.h>

UInt64 SLCPUFrequencyEstimate(void);
bool SLBlockIsCARSystem(OSAddress block);
void SLPrintSystemInfo(CASystemVersionInternal *version);

void SLPrintSystemInfo(CASystemVersionInternal *version)
{
    switch (version->type)
    {
        case kCASystemTypeCoronaX: SLPrintString("Corona-X"); break;
        case kCASystemTypeCorOS:   SLPrintString("CorOS");    break;
        default: SLPrintString("???"); break;
    }

    UInt8 revision = version->revision + 'A';
    UInt32 majorVersion = version->majorVersion;
    SLPrintString(" version %c.%u (", revision, majorVersion);

    switch (version->buildType)
    {
        case kCASystemBuildTypeDebug:       SLPrintString("Debug");         break;
        case kCASystemBuildTypeDevelopment: SLPrintString("Development");   break;
        case kCASystemBuildTypeRelease:     SLPrintString("Release");       break;
        case kCASystemBuildTypeStable:      SLPrintString("Stable");        break;
        default:                            SLPrintString("?????");         break;
    }

    SLPrintString("). Build ID 0x%012X", version->buildID);
}

bool SLBlockIsCARSystem(OSAddress block)
{
    if (XKMemoryCompare(block, kCAHeaderMagic, 4))
        return false;

    return !XKMemoryCompare(block + 4, kCAHeaderVersionSystem, 4);
}

UInt64 SLCPUFrequencyEstimate(void)
{
    UInt64 initialTSC, finalTSC;

    asm ("rdtsc; shrq $32, %%rdx; orq %%rax, %%rdx" : "=d" (initialTSC) : : "rax", "rdx");
    gSLLoaderSystemTable->bootServices->stall(10000);
    asm ("rdtsc; shrq $32, %%rdx; orq %%rax, %%rdx" : "=d" (finalTSC) : : "rax", "rdx");

    if (finalTSC <= initialTSC) return SLCPUFrequencyEstimate();
    return (finalTSC - initialTSC);
}

SLStatus CXSystemLoaderMain(OSUnused OSAddress imageHandle, OSUnused SLSystemTable *systemTable)
{
    // Hmm.... how to start....
    SLPrintString(kSLLoaderWelcomeString);

    UInt64 frequency = SLCPUFrequencyEstimate();
    SLPrintString("CPU Frequency Estimation: %zu\n", frequency);
    SLBootConsoleReadKey(true);

    OSCount count;
    SLBlockIO **blockDevices = SLBlockIOGetAll(&count);

    if (!count) {
        SLPrintString("Aw I didn't find any block devices.\n");
        SLBootConsoleReadKey(true);
        return kSLStatusLoadError;
    } else {
        SLPrintString("I found %u block devices:\n", count);
        OSAddress firstBlock = SLAllocate(512);

        for (OSIndex i = 0; (UInt64)i < count; i++)
        {
            SLPrintString("%zu: ", i);
            if (SLBlockIOReadBlocks(blockDevices[i], 0, firstBlock, 512)) {
                if (SLBlockIsCARSystem(firstBlock)) {
                    SLPrintSystemInfo(&((CAHeaderSystemImage *)firstBlock)->systemVersion);
                    SLPrintString("\n");
                } else {
                    SLPrintString("unknown\n");
                }
            } else {
                SLPrintString("error\n");
            }
        }

        SLBootConsoleReadKey(true);
        SLFree(blockDevices);

        return kSLStatusSuccess;
    }
}
