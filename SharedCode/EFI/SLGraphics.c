#include <SystemLoader/EFI/SLGraphics.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

SLGraphicsOutput **SLGraphicsOutputGetAll(OSCount *count)
{
    return SLBootServicesLocateHandles(kSLGraphicsOutputProtocol, count);
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
