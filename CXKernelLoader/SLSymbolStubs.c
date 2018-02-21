// These symbol stubs are copied over from CXSystemLoader before this binary is executed.
#include <SystemLoader/SLLibrary.h>

// Symbols to hijack beforehand
__attribute__((section("__DATA,__data"))) SLSystemTable *gSLLoaderSystemTable;
__attribute__((section("__DATA,__data"))) OSAddress gSLLoaderImageHandle;
__attribute__((section("__DATA,__data"))) bool gSLBootServicesEnabled;
__attribute__((section("__DATA,__data"))) OSAddress gSLBootXAddress;

// This symbol is always true in this binary regardless of its true-ness
bool gSLBootConsoleIsInitialized = true;
