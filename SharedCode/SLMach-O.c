#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLMach-O.h>

bool SLMachOValidate(OSAddress base, OSSize size);
bool SLMachOReplaceSymbols(OSAddress imageBase, OSSize imageSize, const OSUTF8Char **symbols, OSCount count, const OSAddress *values, OSSize *symbolSizes);

void SLMachOExecute(OSUnused OSAddress base, OSUnused OSSize size)
{
    SLPrintString("Can't load Mach-O binaries yet!!\n");

    return;
}
