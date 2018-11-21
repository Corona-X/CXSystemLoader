#include <SystemLoader/Kernel/SLSMBIOS.h>

SMEntryPoint *SMTableGetMain(void)
{
    return kOSNullPointer;
}

bool SMTableParse(OSUnused SMEntryPoint *entry)
{
    return false;
}
