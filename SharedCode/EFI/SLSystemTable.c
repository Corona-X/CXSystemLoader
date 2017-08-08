#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLLibrary.h>

SLSystemTable *SLSystemTableGetCurrent(void)
{
    return gSLLoaderSystemTable;
}
