#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLLoadedImage.h>
#include <SystemLoader/SLLibrary.h>

SLLoadedImage *SLLoadedImageGetFromHandle(OSAddress imageHandle)
{
    SLBootServicesCheck(kOSNullPointer);

    SLProtocol loadedImageProtocol = kSLLoadedImageProtocol;
    SLLoadedImage *image = kOSNullPointer;

    SLStatus status = SLBootServicesGetCurrent()->handleProtocol(imageHandle, &loadedImageProtocol, &image);
    if (SLStatusIsError(status)) return kOSNullPointer;

    return image;
}

SLFile *SLLoadedImageGetRoot(SLLoadedImage *image)
{
    SLBootServicesCheck(kOSNullPointer);

    SLProtocol volumeProtocol = kSLVolumeProtocol;
    SLVolume *volume = kOSNullPointer;
    SLFile *root = kOSNullPointer;

    SLStatus status = SLBootServicesGetCurrent()->handleProtocol(image->deviceHandle, &volumeProtocol, &volume);
    if (SLStatusIsError(status)) return kOSNullPointer;

    status = volume->openRoot(volume, &root);
    if (SLStatusIsError(status)) return kOSNullPointer;

    return root;
}
