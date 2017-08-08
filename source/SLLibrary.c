#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLConfigFile.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLoader.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <Kernel/XKShared.h>

OSAddress SLGetMainImageHandle(void)
{
    return gSLLoaderImageHandle;
}

SLRuntimeServices *SLRuntimeServicesGetCurrent(void)
{
    return SLSystemTableGetCurrent()->runtimeServices;
}

bool SLDelayProcessor(UIntN time, bool useBootServices)
{
    if (useBootServices && !SLBootServicesHaveTerminated()) {
        SLStatus status = SLBootServicesGetCurrent()->stall(time);
        return !SLStatusIsError(status);
    } else {
        // Try to mimic BS stall() function
        // This only works before MP has been enabled,
        // and the timing is very inacurate....
        for (volatile UInt64 i = 0; i < (time * 100); i++);
        return true;
    }
}

#if kCXBuildDev
    void SLUnrecoverableError(void)
    {
        XKLog(kXKLogLevelFatal, "Unrecoverable Error.\n");
        OSFault();
    }
#else /* !kCXBuildDev */
    void SLUnrecoverableError(void)
    {
        XKLog(kXKLogLevelFatal, "Unrecoverable Error.\n");
        OSFault();
    }
#endif /* kCXBuildDev */
