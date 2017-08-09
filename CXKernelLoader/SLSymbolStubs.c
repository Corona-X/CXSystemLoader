// These symbol stubs are copied over from CXSystemLoader before this binary is executed.
#include <SystemLoader/SLLibrary.h>

SLSystemTable *gSLLoaderSystemTable;
OSAddress gSLLoaderImageHandle;
bool gSLBootServicesEnabled;

// This symbol is always true in this binary regardless of its true-ness
bool gSLBootConsoleIsInitialized = true;
