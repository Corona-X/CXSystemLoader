#include <SystemLoader/EFI/SLGraphics.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

SLGraphicsOutput **SLGraphicsOutputGetAll(OSCount *count)
{
    SLBootServicesCheck(kOSNullPointer);

    SLBootServices *bootServices = SLBootServicesGetCurrent();
    SLProtocol protocol = kSLGraphicsOutputProtocol;
    OSAddress *devices;
    UIntN screenCount;

    SLStatus status = bootServices->localeHandles(kSLSearchTypeByProtocol, &protocol, kOSNullPointer, &screenCount, &devices);
    if (SLStatusIsError(status)) return kOSNullPointer;

    SLGraphicsOutput **results = SLAllocate(screenCount * sizeof(SLGraphicsOutput *)).address;

    for (OSCount i = 0; i < screenCount; i++)
    {
        SLGraphicsOutput *output;
        status = bootServices->handleProtocol(devices[i], &protocol, &output);
        results[i] = output;

        if (SLStatusIsError(status)) goto failure;
    }

    if (!SLBootServicesFree(devices)) goto failure;
    if (count) (*count) = screenCount;
    return results;

failure:
    SLBootServicesFree(devices);
    SLFree(results);
    
    return kOSNullPointer;
}

SLGraphicsModeInfo *SLGraphicsOutputGetMode(SLGraphicsOutput *graphics, UInt32 modeNumber)
{
    SLBootServicesCheck(kOSNullPointer);

    OSSize size = sizeof(SLGraphicsModeInfo *);
    SLGraphicsModeInfo *info;

    SLStatus status = graphics->getMode(graphics, modeNumber, &size, &info);
    bool failure = SLStatusIsError(status);

    return (failure ? kOSNullPointer : info);
}

SLGraphicsMode *SLGraphicsOutputGetCurrentMode(SLGraphicsOutput *graphics)
{
    SLBootServicesCheck(kOSNullPointer);

    return graphics->mode;
}
