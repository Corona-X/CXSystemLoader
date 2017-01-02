#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLFormattedPrint.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLConfigFile.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/XKShared.h>

UInt8 SLEFIInputConsoleRead(bool wait, OSAddress context)
{
    SLStatus status;
    SLKeyPress key;

    do {
        status = SLSystemTableGetCurrent()->stdin->readKey(gSLLoaderSystemTable->stdin, &key);
    } while (wait && status == kSLStatusNotReady);

    return key.keycode;
}

void SLInputConsoleTerminateEFI(void)
{
    SLConsole *previous = kOSNullPointer;
    SLConsole *console = gSLFirstConsole;

    while (console)
    {
        if (console->input == SLEFIInputConsoleRead)
        {
            if (previous) {
                previous->next = console->next;
            } else {
                gSLFirstConsole = console->next;
            }

            SLFree(console);
        }

        previous = console;
        console = console->next;
    }
}

void __SLInputConsoleInitAllEFI(void)
{
    SLBootServicesRegisterTerminationFunction(SLInputConsoleTerminateEFI, kOSNullPointer);
    SLSystemTable *systemTable = SLSystemTableGetCurrent();
    systemTable->stdin->reset(systemTable->stdin, false);

    SLConsole *console = SLAllocate(sizeof(SLConsole)).address;
    console->id = 0xFF;
    console->context = kOSNullPointer;
    console->output = kOSNullPointer;
    console->input = SLEFIInputConsoleRead;
    console->moveBackward = kOSNullPointer;
    console->deleteCharacters = kOSNullPointer;

    SLRegisterConsole(console);
}
