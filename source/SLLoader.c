#include <SystemLoader/SystemLoader.h>

#include <System/Executables/OSELF.h>

SLStatus CXSystemLoaderMain(OSUnused OSAddress imageHandle, OSUnused SLSystemTable *systemTable)
{
    // Hmm.... how to start....
    SLPrintString(kSLLoaderWelcomeString);

    //

    return kSLStatusLoadError;
}
