#include <SystemLoader/Kernel/SLSerialConsole.h>
#include <Kernel/Shared/XKBootConfig.h>
#include <SystemLoader/SLBasicIO.h>

#if kCXBuildDev

bool gSLConsoleIsInitialized = false;

SLSerialConsole gSLSerialConsole;

void SLConsoleInitialize(void)
{
    if (gSLConsoleIsInitialized)
        return;

    gSLSerialConsole.disabled = true;
    gSLConsoleIsInitialized = true;

    // Note that the `gXKBootConfig` symbol is set in the __DATA segment from CXSystemLoader when this binary is loaded.
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

    if (character == '\n')
        XKSerialWriteCharacter(gSLSerialConsole.port, '\r', true);

    XKSerialWriteCharacter(gSLSerialConsole.port, character, true);
}

OSUTF8Char SLSerialConsoleReadKey(bool shouldBlock)
{
    if (gSLSerialConsole.disabled)
        return kXKSerialReadError;

    return XKSerialReadCharacter(gSLSerialConsole.port, shouldBlock);
}

#else /* !kCXBuildDev */

void SLConsoleInitialize(void)
{
    // This stub must exist for CXSystemLoader.
    // It doesn't have to do anything, though...

    return;
}

#endif /* kCXBuildDev */
