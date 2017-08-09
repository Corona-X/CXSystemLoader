#include <SystemLoader/SystemLoader.h>
#include <SystemLoader/SLBasicIO.h>

#include <System/Executables/OSELF.h>

SLStatus CXSystemLoaderMain(OSUnused OSAddress imageHandle, OSUnused SLSystemTable *systemTable)
{
    // Hmm.... how to start....
    SLPrintString(kSLLoaderWelcomeString);
    SLBootConsoleReadKey(true);

    SLLeave(kSLStatusLoadError);
    return kSLStatusLoadError;
}
