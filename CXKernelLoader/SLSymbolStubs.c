// These symbol stubs are copied over directly from CXSystemLoader before this binary is executed.
#include <SystemLoader/SLMemoryAllocator.h>
#include <Kernel/Shared/XKBootConfig.h>
#include <SystemLoader/SLLibrary.h>

// Symbols to hijack beforehand
__attribute__((section("__DATA,__data"))) OSAddress gSLLoaderSystemTable;
__attribute__((section("__DATA,__data"))) OSAddress gSLLoaderImageHandle;
__attribute__((section("__DATA,__data"))) XKBootConfig *gXKBootConfig;
__attribute__((section("__DATA,__data"))) OSAddress gSLBootXAddress;

// Boot Services MUST be enabled when this binary is called.
bool gSLBootServicesEnabled = true;
