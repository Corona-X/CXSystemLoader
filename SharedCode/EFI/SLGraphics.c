#include <SystemLoader/EFI/SLGraphics.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

SLGraphicsOutput **SLGraphicsOutputGetAll(OSCount *count)
{
    SLBootServicesCheck(kOSNullPointer);

    SLBootServices *bootServices = SLBootServicesGetCurrent();
    SLProtocol protocol = kSLGraphicsOutputProtocol;
    OSAddress *devices = kOSNullPointer;
    OSCount screenCount;

    SLStatus status = bootServices->localeHandles(kSLSearchTypeByProtocol, &protocol, kOSNullPointer, &screenCount, &devices);
    if (SLStatusIsError(status)) return kOSNullPointer;

    SLGraphicsOutput **results = SLAllocate(screenCount * sizeof(SLGraphicsOutput *));
    if (!results) goto fail;

    for (OSCount i = 0; i < screenCount; i++)
    {
        status = bootServices->handleProtocol(devices[i], &protocol, &results[i]);
        if (SLStatusIsError(status)) goto fail;
    }

    if (!SLBootServicesFree(devices)) goto fail;
    if (count) (*count) = screenCount;
    return results;

fail:
    if (results) SLFree(results);
    SLBootServicesFree(devices);

    return kOSNullPointer;
}

SLGraphicsModeInfo *SLGraphicsOutputGetMode(SLGraphicsOutput *graphics, UInt32 modeNumber)
{
    SLBootServicesCheck(kOSNullPointer);

    OSSize size = sizeof(SLGraphicsModeInfo *);
    SLGraphicsModeInfo *info = kOSNullPointer;

    SLStatus status = graphics->getMode(graphics, modeNumber, &size, &info);
    if (SLStatusIsError(status)) return kOSNullPointer;

    return info;
}

SLGraphicsMode *SLGraphicsOutputGetCurrentMode(SLGraphicsOutput *graphics)
{
    SLBootServicesCheck(kOSNullPointer);
    return graphics->mode;
}
