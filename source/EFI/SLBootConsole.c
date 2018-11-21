#include <SystemLoader/EFI/SLBootConsole.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLLibrary.h>

#pragma mark - Input Console

SLConsoleInput *SLBootConsoleGetInput(void)
{
    return SLSystemTableGetCurrent()->stdin;
}

bool SLBootConsoleResetInput(void)
{
    SLBootServicesCheck(false);

    SLStatus status = SLBootConsoleGetInput()->reset(SLBootConsoleGetInput(), true);
    return !SLStatusIsError(status);
}

UInt16 SLBootConsoleReadKey(bool shouldBlock)
{
    SLBootServicesCheck(0);
    SLStatus status;
    SLKeyPress key;

    do {
        status = SLBootConsoleGetInput()->readKey(SLBootConsoleGetInput(), &key);
    } while (shouldBlock && status == kSLStatusNotReady);

    return key.keycode;
}

UInt64 SLBootConsoleReadNumber(UInt8 base, bool *success)
{
    if (base != 10)
    {
        // Case not implemented
        SLPrintString("Function %s called with invalid base %u. This functionality is NOT implemented.\n", __func__, base);

        if (success)
            (*success) = false;

        return 0;
    }

    UInt64 result = 0;
    UInt16 keycode;

    while ((keycode = SLBootConsoleReadKey(true)) != '\n')
    {
        if (keycode < '0' || keycode > '9')
        {
            // Invalid input;
            SLPrintString("\nInvalid Input.\n");

            if (success)
                (*success) = false;

            return 0;
        }

        result *= 10;
        result += keycode;
    }

    if (success)
        (*success) = true;

    return result;
}

#pragma mark - Output Console

SLConsoleOutput *SLBootConsoleGetOutput(void)
{
    return SLSystemTableGetCurrent()->stdout;
}

bool SLBootConsoleResetOutput(void)
{
    SLBootServicesCheck(false);

    SLStatus status = SLBootConsoleGetOutput()->reset(SLBootConsoleGetOutput(), true);
    return !SLStatusIsError(status);
}

bool SLBootConsoleOutput(OSUTF16Char *string)
{
    SLBootServicesCheck(false);

    SLStatus status = SLBootConsoleGetOutput()->output(SLBootConsoleGetOutput(), string);
    return !SLStatusIsError(status);
}

bool SLBootConsoleClearScreen(void)
{
    SLBootServicesCheck(false);

    SLStatus status = SLBootConsoleGetOutput()->clearScreen(SLBootConsoleGetOutput());
    return !SLStatusIsError(status);
}
