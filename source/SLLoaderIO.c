#include <SystemLoader/Loader/SLLoaderIO.h>
#include <SystemLoader/EFI/SLBootConsole.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

// This is really 2 pages (UTF-16)
OSUTF16Char gSLPrintBuffer[kSLBootPageSize];
OSIndex gSLPrintBufferIndex = 0;

bool gSLConsoleIsInitialized = false;

void SLConsoleInitialize(void)
{
    SLBootServicesCheck();

    if (gSLConsoleIsInitialized)
        return;

    SLConsoleControl *consoleControl = SLBootServicesLocateProtocol(kSLConsoleControlProtocol);

    if (consoleControl)
    {
        SLStatus status = consoleControl->setMode(consoleControl, kSLConsoleScreenModeText);
        if (SLStatusIsError(status)) SLUnrecoverableError();
    }

    if (!SLBootConsoleResetOutput()) SLUnrecoverableError();
    if (!SLBootConsoleResetInput()) SLUnrecoverableError();
    SLConsoleOutput *stream = SLBootConsoleGetOutput();

    OSIndex maxMode = 0;
    OSSize maxSize = 0;
    SLStatus status;

    for (OSIndex i = 0; ; i++)
    {
        UInt64 columns, rows;

        status = stream->queryMode(stream, i, &columns, &rows);
        if (SLStatusError(status)) break;

        if ((columns * rows) > maxSize)
        {
            maxSize = columns * rows;
            maxMode = i;
        }
    }

    if (!maxSize) SLUnrecoverableError();

    status = stream->setMode(stream, maxMode);
    if (SLStatusIsError(status)) SLUnrecoverableError();

    //status = stream->setAttributes(stream, 0x07);
    //if (SLStatusIsError(status)) SLUnrecoverableError();

    status = stream->enableCursor(stream, false);
    if (SLStatusIsError(status)) SLUnrecoverableError();

    //if (!SLBootConsoleClearScreen())
    //    SLUnrecoverableError();

    gSLConsoleIsInitialized = true;
}

// Just Print to UEFI
// This uses a buffer which is flushed on newlines
void SLPrintCharacter(const OSUTF8Char character)
{
    gSLPrintBuffer[gSLPrintBufferIndex++] = character;

    if (character == '\n')
        SLPrintCharacter('\r');

    if (gSLPrintBufferIndex >= (kSLBootPageSize - 1) || character == '\n')
        SLPrintBufferFlush();
}

void SLPrintBufferFlush(void)
{
    gSLPrintBuffer[gSLPrintBufferIndex] = 0;
    gSLPrintBufferIndex = 0;

    SLBootConsoleOutput(gSLPrintBuffer);
}
