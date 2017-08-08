#include <SystemLoader/EFI/SLLoadedImage.h>
#include <SystemLoader/SLLibrary.h>

SLLoadedImage *SLLoadedImageGetFromHandle(OSAddress imageHandle)
{
    SLBootServicesCheck(kOSNullPointer);

    SLProtocol loadedImageProtocol = kSLLoadedImageProtocol;
    SLLoadedImage *image = kOSNullPointer;

    SLStatus status = SLBootServicesGetCurrent()->handleProtocol(imageHandle, &loadedImageProtocol, &image);
    bool failed = SLStatusIsError(status);

    return (failed ? kOSNullPointer : image);
}

SLFile *SLLoadedImageGetRoot(SLLoadedImage *image)
{
    SLBootServicesCheck(kOSNullPointer);

    SLProtocol volumeProtocol = kSLVolumeProtocol;
    SLVolume *volume = kOSNullPointer;

    SLStatus status = SLBootServicesGetCurrent()->handleProtocol(image->deviceHandle, &volumeProtocol, &volume);
    if (SLStatusIsError(status)) return kOSNullPointer;

    SLFile *root;
    status = volume->openRoot(volume, &root);
    bool failed = SLStatusIsError(status);

    return (failed ? kOSNullPointer : root);
}
