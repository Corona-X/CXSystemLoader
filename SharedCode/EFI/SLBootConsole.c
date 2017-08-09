#include <SystemLoader/EFI/SLBootConsole.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLLibrary.h>

bool SLBootConsoleResetInput(void)
{
    SLBootServicesCheck(false);

    SLStatus status = SLSystemTableGetCurrent()->stdin->reset(SLSystemTableGetCurrent()->stdin, true);
    return !SLStatusIsError(status);
}

UInt16 SLBootConsoleReadKey(bool shouldBlock)
{
    SLBootServicesCheck(0);
    SLStatus status;
    SLKeyPress key;

    do {
        status = SLSystemTableGetCurrent()->stdin->readKey(SLSystemTableGetCurrent()->stdin, &key);
    } while (shouldBlock && status == kSLStatusNotReady);

    return key.keycode;
}

bool SLBootConsoleResetOutput(void)
{
    SLBootServicesCheck(false);

    SLStatus status = SLSystemTableGetCurrent()->stdout->reset(SLSystemTableGetCurrent()->stdout, true);
    return !SLStatusIsError(status);
}

bool SLBootConsoleOutput(OSUTF16Char *string)
{
    SLBootServicesCheck(false);

    SLStatus status = SLSystemTableGetCurrent()->stdout->output(SLSystemTableGetCurrent()->stdout, string);
    return !SLStatusIsError(status);
}

bool SLBootConsoleGetMode(UInt64 mode, UInt64 *columns, UInt64 *rows);

bool SLBootConsoleSetMode(UInt64 mode);

bool SLBootConsoleClearScreen(void);

bool SLBootConsoleEnableCursor(bool enable);
