#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLMemoryAllocator.h>

OSAddress SLGetMainImageHandle(void)
{
    return gSLLoaderImageHandle;
}

bool SLDelayProcessor(UInt64 time)
{
    if (!SLBootServicesHaveTerminated()) {
        SLStatus status = SLBootServicesGetCurrent()->stall(time);
        return !SLStatusIsError(status);
    } else {
        // Try to mimic BS stall() function
        // This only works before MP has been enabled,
        // and the timing is very inacurate....
        for (__volatile__ UInt64 i = 0; i < (time * 100); i++);
        return true;
    }
}

#if kCXBuildDev
    void SLUnrecoverableError(void)
    {
        if (gSLConsoleIsInitialized)
        {
            SLPrintString("Unrecoverable Error!\n\n");

            SLPrintString("Memory Allocator State:|n");
            SLMemoryAllocatorDumpHeapInfo();
            SLMemoryAllocatorDumpMainPool();
        }

        SLLeave(kSLStatusLoadError);
    }
#else /* !kCXBuildDev */
    void SLUnrecoverableError(void)
    {
        if (gSLConsoleIsInitialized)
            SLPrintString("Unrecoverable Error!\n");

        SLLeave(kSLStatusLoadError);
    }
#endif /* kCXBuildDev */
