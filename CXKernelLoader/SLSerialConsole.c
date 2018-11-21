#include <SystemLoader/Kernel/SLSerialConsole.h>
#include <Kernel/Shared/XKBootConfig.h>
#include <SystemLoader/SLBasicIO.h>

bool gSLConsoleIsInitialized = false;

SLSerialConsole gSLSerialConsole;

void SLConsoleInitialize(void)
{
    if (gSLConsoleIsInitialized)
        return;

    gSLSerialConsole.disabled = true;
    gSLConsoleIsInitialized = true;

    XKSerialPort prospectivePort = XKBootConfigGetNumber(gXKBootConfig, kXKBootConfigKeySerialPort, kXKSerialDefaultPort);
    SInt64 prospectiveRate = XKBootConfigGetNumber(gXKBootConfig, kXKBootConfigKeySerialRate, kXKSerialDefaultSpeed);

    XKSerialPort port = prospectivePort & 0xFFFF;
    UInt32 rate = prospectiveRate & 0xFFFFFFFF;

    if (prospectiveRate != rate)
        rate = kXKSerialDefaultSpeed;

    if (prospectivePort != port)
        port = kXKSerialDefaultPort;

    if ((gSLSerialConsole.port = XKSerialPortInit(port)) == kXKSerialPortError)
    {
        gSLSerialConsole.disabled = true;
        return;
    }

    if (!XKSerialPortSetBaudRate(gSLSerialConsole.port, rate)) {
        if (!XKSerialPortSetBaudRate(gSLSerialConsole.port, kXKSerialDefaultSpeed))
        {
            gSLSerialConsole.port = kXKSerialPortError;
            return;
        }

        gSLSerialConsole.rate = kXKSerialDefaultSpeed;
    } else {
        gSLSerialConsole.rate = rate;
    }

    XKSerialPortSetupLineControl(gSLSerialConsole.port, kXKSerialWordLength8Bits, kXKSerialNoParity, kXKSerial1StopBit);

    gSLSerialConsole.disabled = false;
}

void SLPrintCharacter(const OSUTF8Char character)
{
    if (gSLSerialConsole.disabled)
        return;

    XKSerialWriteCharacter(gSLSerialConsole.port, character, true);
}

OSUTF8Char SLSerialConsoleReadKey(bool shouldBlock)
{
    return XKSerialReadCharacter(gSLSerialConsole.port, shouldBlock);
}
