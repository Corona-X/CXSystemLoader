#include <SystemLoader/SLFormattedPrint.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLConfigFile.h>
#include <Kernel/XKSerial.h>
#include <Kernel/XKShared.h>

void SLSerialConsoleOutput(OSUTF8Char *string, OSSize size, XKSerialPort port)
{
    for (OSIndex i = 0; i < size; i++)
    {
        if (string[i] == '\n')
            XKSerialWriteCharacter(port, '\r', true);

        XKSerialWriteCharacter(port, string[i], true);
    }
}

UInt8 SLSerialConsoleInput(bool wait, XKSerialPort port)
{
    UInt8 character = XKSerialReadCharacter(port, wait);

    if (character == kXKSerialReadError)
    {
        SLPrintString("Warning: Detected serial data transmission error!\n");
        character = 0;
    }

    return character;
}

void SLSerialConsoleMoveBackward(OSCount spaces, XKSerialPort port)
{
    for (OSCount i = 0; i < spaces; i++)
        XKSerialWriteCharacter(port, '\b', true);
}

void SLSerialConsoleDeleteCharacters(OSCount spaces, XKSerialPort port)
{
    for (OSCount i = 0; i < spaces; i++)
        XKSerialWriteString(port, (UInt8 *)"\b \b");
}

void __SLSerialConsoleInitAll(void)
{
    SLConfigFile *config = SLConfigGet();

    if (!config->dev.serialConsole.enabled)
        return;

    for (OSIndex i = 0; i < config->dev.serialConsole.portCount; i++)
    {
        XKSerialPort port = XKSerialPortInit(config->dev.serialConsole.ports[i]);

        if (port == kXKSerialPortError)
        {
            XKPrintString("Error Loading Serial Port at '0x%04X'\n", config->dev.serialConsole.ports[i]);
            continue;
        }

        XKSerialPortSetupLineControl(port, config->dev.serialConsole.worldLength, config->dev.serialConsole.parityType, config->dev.serialConsole.stopBits);
        XKSerialPortSetBaudRate(port, config->dev.serialConsole.baudRate);

        SLConsole *console = SLAllocate(sizeof(SLConsole)).address;

        console->id = 0xFF;
        console->context = port;
        console->output = SLSerialConsoleOutput;
        console->input = SLSerialConsoleInput;
        console->moveBackward = SLSerialConsoleMoveBackward;
        console->deleteCharacters = SLSerialConsoleDeleteCharacters;

        SLRegisterConsole(console);
    }
}
