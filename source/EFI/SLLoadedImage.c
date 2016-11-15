#include <SystemLoader/SystemLoader.h>

SLLoadedImage *SLLoadedImageGetFromHandle(OSAddress imageHandle)
{
    SLProtocol loadedImageProtocol = kSLLoadedImageProtocol;
    SLLoadedImage *image = kOSNullPointer;

    SLStatus status = gSLLoaderSystemTable->bootServices->handleProtocol(imageHandle, &loadedImageProtocol, &image);
    bool failed = SLStatusIsError(status);

    return (failed ? kOSNullPointer : image);
}

SLFile *SLLoadedImageGetRoot(SLLoadedImage *image)
{
    SLProtocol volumeProtocol = kSLVolumeProtocol;
    SLVolume *volume = kOSNullPointer;

    SLStatus status = gSLLoaderSystemTable->bootServices->handleProtocol(image->deviceHandle, &volumeProtocol, &volume);
    if (SLStatusIsError(status)) return kOSNullPointer;

    SLFile *root;
    status = volume->openRoot(volume, &root);
    bool failed = SLStatusIsError(status);

    return (failed ? kOSNullPointer : root);
}
