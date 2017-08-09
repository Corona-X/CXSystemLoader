#include <SystemLoader/EFI/SLSystemTable.h>
#include <SystemLoader/SLBasicIO.h>
#include <System/OSByteMacros.h>

OSPrivate static void SLPrintNumber(UInt64 *n, bool isSigned, UInt8 base, UInt8 padding);

// This is really 2 pages (UTF-16)
OSUTF16Char gSLPrintBuffer[kSLBootPageSize];
OSIndex gSLPrintBufferIndex = 0;

// Just Print to UEFI (with a buffer)
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

static void SLPrintNumber(UInt64 *n, bool isSigned, UInt8 base, UInt8 padding)
{
    OSUTF8Char buffer[64];
    UInt64 number;
    SInt8 i = 0;

    if (isSigned) {
        // TODO: Are these numbers processed properly?

        if (n < 0) {
            SLPrintCharacter('-');
            number = -(*((SInt64 *)n));
        } else {
            number = +(*((SInt64 *)n));
        }
    } else {
        number = (*n);
    }

    if (!number)
    {
        if (padding)
        {
            for (UInt8 written = 0; written < padding; written++)
                SLPrintCharacter('0');
        }

        return;
    }

    do {
        UInt8 next = number % base;
        number /= base;

        if (next < 10) {
            buffer[i++] = next + '0';
        } else if (next < 37) {
            buffer[i++] = (next - 10) + 'A';
        } else if (next < 63) {
            buffer[i++] = (next - 36) + 'a';
        } else if (next < 64) {
            buffer[i++] = '+';
        } else if (next < 65) {
            buffer[i++] = '/';
        } else {
            // Overflow character is '#'
            // only possible with base >= 65
            buffer[i++] = '#';
        }
    } while (number);

    if (i < padding)
    {
        UInt8 padCount = (padding - i);

        for (UInt8 written = 0; written < padCount; written++)
            SLPrintCharacter('0');
    }

    for (i--; i >= 0; i--)
        SLPrintCharacter(buffer[i]);
}

// This 'printf' function is nowhere near compliant to any standard.
// It's the bootloader, who cares.
void SLPrintString(OSUnused const OSUTF8Char *format, ...)
{
    #if !kCXBuildDev
        if (OSUnlikely(!gSLBootConsoleIsInitialized))
            SLBootConsoleInitialize();
    #endif /* kCXBuildDev */

    bool justEnteredEscapeCode = false;
    bool specialPadding = false;
    bool inEscapeCode = false;
    bool printNumber = false;
    bool printSign = false;
    bool inPadding = false;
    bool hexDump = false;

    UInt8 argSize = 4;
    UInt8 padding = 1;
    UInt8 base = 10;
    OSVAList args;
    UInt8 c;

    OSVAStart(args, format);

    while ((c = (*format++)))
    {
        if (!inEscapeCode)
        {
            if (c == '%') {
                justEnteredEscapeCode = true;
                inEscapeCode = true;
            } else {
                SLPrintCharacter(c);
            }

            continue;
        }

        if (justEnteredEscapeCode)
        {
            if (c == '%')
            {
                SLPrintCharacter('%');
                inEscapeCode = false;

                continue;
            }
        }

        // Yes, I realize that this will allow an
        // infinite number of size flags to be
        // used, and that the last size flag given
        // will take precedence over all others.
        // This doesn't matter, however, it doesn't
        // actually expose anything that could be
        // exploited.
        switch (c)
        {
            case '0': {
                specialPadding = true;
                inPadding = true;
                padding = 0;
            } break;
            case 'h': {
                argSize = 2;
            } break;
            case 'l':
            case 'z': {
                argSize = 8;
            } break;
            // Extension to print binary
            // numbers of length n
            case 'b': {
                printNumber = true;
                base = 2;
            } break;
            case 'o': {
                printNumber = true;
                base = 8;
            } break;
            case 'u': {
                printNumber = true;
                base = 10;
            } break;
            case 'd': {
                printNumber = true;
                printSign = true;
                base = 10;
            } break;
            case 'p':
                SLPrintCharacter('0');
                SLPrintCharacter('x');

                argSize = 8;
            case 'x':
            case 'X': {
                printNumber = true;
                base = 16;
            } break;
            case 'c': {
                inEscapeCode = false;

                OSUTF8Char character = OSVAGetNext(args, UInt32);
                SLPrintCharacter(character);
            } break;
            case 's': {
                const OSUTF8Char *string = OSVAGetNext(args, OSUTF8Char *);
                inEscapeCode = false;

                if (specialPadding) {
                    for (UInt8 i = 0; i < padding; i++)
                    {
                        OSUTF8Char character = *string++;
                        if (!character) break;

                        SLPrintCharacter(character);
                    }

                    specialPadding = false;
                    padding = 1;
                } else {
                    while (*string)
                        SLPrintCharacter(*string++);
                }
            } break;
            case 'S': {
                // Ehh this probably doesn't have to be implemented...
            } break;
            // Extension to print a hex string
            // from a given address
            //
            // The length of the string is
            // specified by the padding,
            // and the number of characters
            // in between spaces is specified
            // by the length modifiers used
            //
            // Because of the implementation
            // of the padding, the maximum
            // amount of data which can be
            // dumped is 255 pieces of the
            // given data size. This gives
            // an overall maximum of 2040
            // bytes with a padding of 255
            // and a 64-bit separator size
            case 'H': {
                hexDump = true;
            } break;
        }

        if (inPadding)
        {
            if (OSIsBetween('0', c, '9')) {
                padding *= 10;
                padding += (c - '0');
            } else {
                inPadding = false;
            }
        }

        if (hexDump)
        {
            OSAddress argument = OSVAGetNext(args, OSAddress);
            UInt64 number = 0;

            while (padding)
            {
                #define entry(c, l)                     \
                    case c: {                           \
                        UInt ## l *arg = argument;      \
                        number = (*arg);                \
                        argument += sizeof(UInt ## l);  \
                    } break

                switch (argSize)
                {
                    entry(2, 16);
                    entry(4, 32);
                    entry(8, 64);
                }

                SLPrintNumber(&number, false, 16, (argSize * 2));

                if (padding != 1)
                    SLPrintCharacter(' ');

                #undef entry
                padding--;
            }

            specialPadding = false;
            inEscapeCode = false;
            hexDump = false;

            argSize = 4;
            padding = 1;
        }

        if (printNumber)
        {
            SInt64 number = 0;

            #define entry(p)                                    \
                switch (argSize)                                \
                {                                               \
                    case 2:                                     \
                    case 4: {                                   \
                        number = OSVAGetNext(args, p ## Int32); \
                    } break;                                    \
                    case 8: {                                   \
                        number = OSVAGetNext(args, p ## Int64); \
                    } break;                                    \
                }

            if (!printSign) {
                entry(U);
            } else {
                entry(S);
            }

            SLPrintNumber((UInt64 *)&number, printSign, base, padding);
            #undef entry

            inEscapeCode = false;
            printNumber = false;
            printSign = false;
            padding = 1;
        }
    }

    OSVAFinish(args);
}
