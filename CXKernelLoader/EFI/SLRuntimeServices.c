#include <SystemLoader/EFI/SLRuntimeServices.h>
#include <SystemLoader/EFI/SLSystemTable.h>

SLRuntimeServices *SLRuntimeServicesGetCurrent(void)
{
    return SLSystemTableGetCurrent()->runtimeServices;
}
