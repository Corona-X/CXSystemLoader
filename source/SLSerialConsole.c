#include <SystemLoader/SystemLoader.h>

typedef void (*SLConsoleOutput)(OSUTF8Char *string, OSSize size, OSAddress context);
typedef UInt8 (*SLConsoleInput)(bool wait, OSAddress context);
typedef void (*SLConsoleMoveBackward)(OSCount spaces, OSAddress context);
typedef void (*SLConsoleDeleteCharacters)(OSCount characters, OSAddress context);

void SLSerialConsoleOutput(OSUTF8Char *string, OSSize size, SLSerialPort port)
{
    for (OSIndex i = 0; i < size; i++)
        SLSerialWriteCharacter(port, string[i], true);
}

UInt8 SLSerialConsoleInput(bool wait, SLSerialPort port)
{
    return SLSerialReadCharacter(port, wait);
}

void SLSerialConsoleMoveBackward(OSCount spaces, SLSerialPort port)
{
    for (OSCount i = 0; i < spaces; i++)
        SLSerialWriteCharacter(port, '\b', true);
}

void SLSerialConsoleDeleteCharacters(OSCount spaces, SLSerialPort port)
{
    for (OSCount i = 0; i < spaces; i++)
        SLSerialWriteString(port, (UInt8 *)"\b \b");
}

void __SLSerialConsoleInitAll(void)
{
    SLConfigFile *config = SLConfigGet();

    if (!config->dev.serialConsole.enabled)
        return;

    for (OSIndex i = 0; i < config->dev.serialConsole.portCount; i++)
    {
        SLSerialPort port = SLSerialPortInit(config->dev.serialConsole.ports[i]);

        if (port == kSLSerialPortError)
        {
            SLPrintString("Error Loading Serial Port at '0x%04X'\n", port);
            continue;
        }

        SLSerialPortSetupLineControl(port, config->dev.serialConsole.worldLength, config->dev.serialConsole.parityType, config->dev.serialConsole.stopBits);
        SLSerialPortSetBaudRate(port, config->dev.serialConsole.baudRate);

        SLConsole *console = SLAllocate(sizeof(SLConsole)).address;

        console->id = 0xFF;
        console->context = port;
        console->output = SLSerialConsoleOutput;
        console->input = SLSerialConsoleInput;
        console->height = console->width = 0;
        console->moveBackward = SLSerialConsoleMoveBackward;
        console->deleteCharacters = SLSerialConsoleDeleteCharacters;

        SLRegisterConsole(console);
    }
}
