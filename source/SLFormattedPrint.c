#include <SystemLoader/SystemLoader.h>
#include <System/OSCompilerMacros.h>
#include <SystemLoader/SLLibrary.h>

#define kSLScanBufferSize 1024

static const OSUnicodePoint kSLSurrogateHighBegin  = 0xD800;
static const OSUnicodePoint kSLSurrogateHighFinish = 0xDBFF;
static const OSUnicodePoint kSLSurrogateLowBegin   = 0xDC00;
static const OSUnicodePoint kSLSurrogateLowFinish  = 0xDFFF;

static const UInt8 kSLUTF8ExtraByteCount[0x100] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};

static const UInt64 kSLUTF8Excess[6] = {
    0x0000000000,
    0x0000003040,
    0x00000E2080,
    0x00E2041040,
    0x00FA082080,
    0x3F82082080
};

static const OSUTF8Char kSLUTF8BitMasks[7] = {
    0,
    0b00000000,
    0b11000000,
    0b11100000,
    0b11110000,
    0b11111000,
    0b11111100
};

OSPrivate OSCount SLUTF8FromCodePoint(OSUnicodePoint point, OSUTF8Char *output, OSCount count)
{
    #define next(o, b, p)           \
        b[o] = (p | 0x80) & 0xBF;   \
        p >>= 6;                    \

    #define finish(o, p, c) o[0] = p | kSLUTF8BitMasks[c]

    #define entry(b, c, p, o, entries)  \
        do {                            \
            if (c < b) return c;        \
                                        \
            entries                     \
            finish(o, p, b);            \
                                        \
            return (b - 1);             \
        } while (0)

    if (point < 0x80) {
        (*output) = (OSUTF8Char)point;
        return 0;
    } else if (point < 0x800) {
        entry(2, count, point, output, {
            next(1, output, point);
        });
    } else if (point < 0x10000) {
        entry(3, count, point, output, {
            next(2, output, point);
            next(1, output, point);
        });
    } else if (point < 0x200000) {
        entry(4, count, point, output, {
            next(3, output, point);
            next(2, output, point);
            next(1, output, point);
        });
    } else {
        entry(5, count, point, output, {
            next(4, output, point);
            next(3, output, point);
            next(2, output, point);
            next(1, output, point);
        });
    }

    #undef entry
    #undef finish
    #undef next
}

OSPrivate OSUnicodePoint SLUTF8ToCodePoint(OSUTF8Char *input, OSCount inCount, OSCount *usedCount)
{
    OSCount count = kSLUTF8ExtraByteCount[*input];
    OSUnicodePoint point = 0;

    if (inCount < count)
    {
        (*usedCount) = inCount;

        return kOSUTF32Error;
    }

    switch (count)
    {
        case 5:
            point += (*input++);
            point <<= 6;
        case 4:
            point += (*input++);
            point <<= 6;
        case 3:
            point += (*input++);
            point <<= 6;
        case 2:
            point += (*input++);
            point <<= 6;
        case 1:
            point += (*input++);
            point <<= 6;
        case 0:
            point += (*input++);
    }

    point -= kSLUTF8Excess[count];
    (*usedCount) = (count);

    return point;
}

OSPrivate OSCount SLUTF16FromCodePoint(OSUnicodePoint point, OSUTF16Char *output, OSCount count)
{
    if (point < 0x10000) {
        (*output) = point;
    } else if (count) {
        point -= 0x10000;

        output[0] = (point >> 10)    + kSLSurrogateHighBegin;
        output[1] = (point & 0x3FFF) + kSLSurrogateLowBegin;

        return 1;
    }

    return 0;
}

OSPrivate OSUnicodePoint SLUTF16ToCodePoint(OSUTF16Char *input, OSCount inCount, OSCount *usedCount)
{
    OSUTF16Char first = (*input++);
    (*usedCount) = 0;

    if (OSIsBetween(kSLSurrogateHighBegin, first, kSLSurrogateHighFinish)) {
        if (!inCount) return kOSUTF32Error;

        OSUTF16Char second = (*input);
        (*usedCount) = 1;

        if (OSIsBetween(kSLSurrogateLowBegin, second, kSLSurrogateLowFinish)) {
            return ((OSUnicodePoint)((first << 10) | second) + 0x10000);
        } else {
            return kOSUTF32Error;
        }
    } else if (OSIsBetween(kSLSurrogateLowBegin, first, kSLSurrogateLowFinish)) {
        return kOSUTF32Error;
    } else {
        return ((OSUnicodePoint)first);
    }
}

OSSize SLUTF16SizeInUTF8(OSUTF16Char *utf16)
{
    OSLength stringLength = CXKStringSize16(utf16);
    OSSize size = 0;

    for (OSCount i = 0; i < stringLength; i++)
    {
        OSUTF16Char character = utf16[i];

        if (OSIsBetween(kSLSurrogateHighBegin, character, kSLSurrogateHighFinish)) {
            if (!((i++) < stringLength)) return kOSSizeError;
            character = utf16[i];

            if (OSIsBetween(kSLSurrogateLowBegin, character, kSLSurrogateLowFinish)) {
                size += 2;
            } else {
                return kOSSizeError;
            }
        } else if (OSIsBetween(kSLSurrogateLowBegin, character, kSLSurrogateLowFinish)) {
            return kOSSizeError;
        } else {
            size++;
        }
    }

    return size;
}

OSSize SLUTF8SizeInUTF16(OSUTF8Char *utf8)
{
    OSLength stringLength = CXKStringSize8(utf8);
    OSSize size = 0;

    for (OSCount i = 0; i < stringLength; i++)
    {
        UInt8 extraBytes = kSLUTF8ExtraByteCount[utf8[i]];
        i += (extraBytes - 1);

        if (extraBytes && (i >= stringLength))
            return kOSSizeError;

        if (extraBytes >= 2) {
            size += 2;
        } else {
            size++;
        }
    }

    return size;
}

OSUTF8Char *SLUTF16ToUTF8(OSUTF16Char *utf16)
{
    OSSize utf8size = SLUTF16SizeInUTF8(utf16);
    if (utf8size == kOSSizeError) return kOSNullPointer;

    OSUTF8Char *result = SLAllocate((utf8size + 1) * sizeof(OSUTF8Char)).address;
    OSUTF8Char *utf8 = result;

    OSCount utf16size = CXKStringSize16(utf16);
    OSUTF8Char *end = result + utf8size;
    OSCount used;

    while (utf8 < end)
    {
        OSUnicodePoint codepoint = SLUTF16ToCodePoint(utf16, utf16size, &used);
        utf16 += (used + 1);

        used = SLUTF8FromCodePoint(codepoint, utf8, utf8size);
        utf8 += (used + 1);
    }

    (*utf8) = 0;
    return result;
}

OSUTF16Char *SLUTF8ToUTF16(OSUTF8Char *utf8)
{
    OSSize utf16size = SLUTF8SizeInUTF16(utf8);
    if (utf16size == kOSSizeError) return kOSNullPointer;

    OSUTF16Char *result = SLAllocate((utf16size + 1) * sizeof(OSUTF16Char)).address;
    OSUTF16Char *utf16 = result;

    OSCount utf8size = CXKStringSize8(utf8);
    OSUTF16Char *end = result + utf16size;
    OSCount used;

    while (result != end)
    {
        OSUnicodePoint codepoint = SLUTF8ToCodePoint(utf8, utf8size, &used);
        utf8 += (used + 1);

        used = SLUTF16FromCodePoint(codepoint, utf16, utf16size);
        utf16 += (used + 1);
    }

    (*utf16) = 0;
    return result;
}

SLConsole *gSLFirstConsole = kOSNullPointer;

SInt8 SLRegisterConsole(SLConsole *console)
{
    console->next = kOSNullPointer;

    if (!gSLFirstConsole) {
        gSLFirstConsole = console;
        console->id = 0;
    } else {
        SLConsole *last = gSLFirstConsole;

        while (last->next)
            last = last->next;

        console->id = last->id + 1;
        last->next = console;
    }

    return console->id;
}

void SLMoveBackward(OSCount count)
{
    SLConsole *console = gSLFirstConsole;

    while (console)
    {
        if (console->moveBackward)
            console->moveBackward(count, console->context);

        console = console->next;
    }
}

void SLDeleteCharacters(OSCount count)
{
    SLConsole *console = gSLFirstConsole;

    while (console)
    {
        if (console->deleteCharacters) {
            console->deleteCharacters(count, console->context);
        } else if (console->moveBackward && console->output) {
            console->moveBackward(count, console->context);
            OSUTF8Char *spaces = SLAllocate(count).address;
            CXKMemorySetValue(spaces, count, ' ');
            console->output(spaces, count, console->context);
            SLFree(spaces);
            console->moveBackward(count, console->context);
        }

        console = console->next;
    }
}

UInt8 *SLScanString(UInt8 terminator, OSSize *size)
{
    UInt8 string[kSLScanBufferSize];
    OSIndex i = 0;

    for ( ; ; )
    {
        SLConsole *console = gSLFirstConsole;

        while (console)
        {
            if (console->input)
            {
                UInt8 read = console->input(false, console->context);

                if (read != kOSUTF8Error)
                {
                    if (read == terminator) {
                        string[i++] = 0;

                        OSUTF8Char *result = SLAllocate(i).address;
                        CXKMemoryCopy(string, result, i);
                        return result;
                    } else {
                        string[i++] = read;
                    }
                }
            }

            console = console->next;
        }
    }
}

OSInline void SLPrintChars(OSUTF8Char *source, OSCount count)
{
    SLConsole *console = gSLFirstConsole;

    while (console)
    {
        if (console->output)
            console->output(source, count, console->context);

        console = console->next;
    }
}

OSUTF8Char *SLUIDToString(SLProtocol *uid)
{
    return SLPrintToString("%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
               uid->group0, uid->group1, uid->group2,
               uid->group3[0], uid->group3[1], uid->group3[2], uid->group3[3],
               uid->group3[4], uid->group3[5], uid->group3[6], uid->group3[7]);
}

void SLNumberToStringConverter(SInt64 n, bool isSigned, UInt8 base, UInt8 padding, UInt8 hexStart, bool shouldPrint, OSLength *length, OSUTF8Char **string)
{
    if (!shouldPrint && !string)
        SLUnrecoverableError();

    bool prependNegative = false;
    OSUTF8Char buffer[32];
    UInt8 offset = 0;
    UInt64 number;
    SInt8 i = 0;

    if (isSigned && (n < 0)) {
        prependNegative = true;
        number = -n;
    } else {
        number = n;
    }

    if (!number)
    {
        if (!shouldPrint && length) (*length) = padding;

        if (padding) {
            if (!shouldPrint) {
                (*string) = SLAllocate(padding + 1).address;
                CXKMemorySetValue((*string), padding, '0');
                (*string)[padding] = 0;
            } else {
                OSUTF8Char *zeros = SLAllocate(padding).address;
                CXKMemorySetValue(zeros, padding, '0');
                SLPrintChars(zeros, padding);
                SLFree(zeros);
            }
        } else {
            if (!shouldPrint)
            {
                (*string) = SLAllocate(1).address;
                (*string)[0] = 0;
            }
        }

        return;
    }

    do {
        UInt8 next = number % base;
        number /= base;
        
        if (next < 10) {
            buffer[i++] = next + '0';
        } else if (next < 37) {
            buffer[i++] = (next - 10) + hexStart;
        } else {
            buffer[i++] = '#';
        }
    } while (number);

    if (i < padding) {
        UInt8 zeroCount = (padding - i);

        if (!shouldPrint) {
            if (length) (*length) = padding;

            (*string) = SLAllocate(padding + 1).address;
            (*string)[padding] = 0;

            if (prependNegative) {
                CXKMemorySetValue((*string) + 1, zeroCount - 1, '0');
                (*string)[0] = '-';
            } else {
                CXKMemorySetValue((*string), zeroCount, '0');
            }

            offset = zeroCount;
        } else {
            if (prependNegative)
            {
                SLPrintChars((OSUTF8Char *)"-", 1);
                zeroCount--;
            }

            OSUTF8Char *zeros = SLAllocate(zeroCount).address;
            CXKMemorySetValue(zeros, zeroCount, '0');
            SLPrintChars(zeros, zeroCount);
            SLFree(zeros);
        }
    } else {
        if (prependNegative) {
            if (!shouldPrint) {
                if (length) (*length) = (i + 1);

                (*string) = SLAllocate(i + 2).address;
                (*string)[i + 1] = 0;
                (*string)[0] = '-';
                offset = 1;
            } else {
                SLPrintChars((OSUTF8Char *)"-", 1);
            }
        } else {
            if (!shouldPrint)
            {
                if (length) (*length) = i;
                
                (*string) = SLAllocate(i + 1).address;
                (*string)[i] = 0;
                offset = 0;
            }
        }
    }

    if (!shouldPrint) {
        for (i--; i >= 0; i--)
            string[offset++] = buffer[i];
    } else {
        OSUTF8Char finalBuffer[30];
        UInt8 toCopy = i;
        offset = 0;

        for (i--; i >= 0; i--)
            finalBuffer[offset++] = buffer[i];

        SLPrintChars(finalBuffer, toCopy);
    }
}

OSUTF8Char *SLNumberToString(SInt64 n, bool isSigned, UInt8 base, UInt8 padding, UInt8 hexStart, OSLength *length)
{
    OSUTF8Char *string;

    SLNumberToStringConverter(n, isSigned, base, padding, hexStart, false, length, &string);

    return string;
}

OSUTF8Char *SLPrintToString(const char *format, ...)
{
    OSVAList args;
    OSVAStart(args, format);
    OSUTF8Char *result = SLPrintToStringFromList(format, args);
    OSVAFinish(args);

    return result;
}

void SLPrintString(const char *format, ...)
{
    OSVAList args;
    OSVAStart(args, format);
    SLPrintStringFromList(format, args);
    OSVAFinish(args);
}

void SLConsoleCountOutputLength(OSUTF8Char *string, OSLength length, OSLength *previous)
{
    (*previous) += length;
}

void SLConsolePrintToStringOutput(OSUTF8Char *newChars, OSLength length, OSUTF8Char **string)
{
    CXKMemoryCopy(newChars, (*string), length);
    (*string) += length;
}

OSUTF8Char *SLPrintToStringFromList(const char *format, OSVAList args)
{
    OSVAList *copy = kOSNullPointer;
    OSVACopy(copy, args);
    OSLength length;

    SLConsole *firstConsole = gSLFirstConsole;
    SLConsole lengthConsole;
    lengthConsole.output = SLConsoleCountOutputLength;
    lengthConsole.context = &length;
    gSLFirstConsole = &lengthConsole;
    SLPrintStringFromList(format, copy);
    length++;

    OSUTF8Char *string = SLAllocate(length).address;
    OSUTF8Char *stringCopy = string;

    SLConsole printConsole;
    printConsole.output = SLConsolePrintToStringOutput;
    printConsole.context = &stringCopy;
    gSLFirstConsole = &printConsole;
    SLPrintStringFromList(format, args);
    string[length] = 0;

    gSLFirstConsole = firstConsole;
    return string;
}

#if !kCXBuildDev
    #undef SLPrintString
#endif /* !kCXBuildDev */

void SLPrintStringFromList(const char *format, OSVAList args)
{
    bool justEnteredEscapeCode = false;
    bool inEscapeCode = false;
    bool printNumber = false;
    bool printSign = false;
    bool inPadding = false;
    bool hexDump = false;

    UInt8 hexBase = 'A';
    UInt8 lastFlag = 0;
    UInt8 argSize = 4;
    UInt8 padding = 1;
    UInt8 base = 10;

    OSCount charsToPrint = 0;

    while (*format)
    {
        UInt8 c = (*format++);

        if (!inEscapeCode)
        {
            if (c == '%'){
                justEnteredEscapeCode = true;
                inEscapeCode = true;
            } else {
                charsToPrint++;
            }

            continue;
        }

        if (justEnteredEscapeCode)
        {
            justEnteredEscapeCode = false;

            if (c == '%')
            {
                inEscapeCode = false;
                charsToPrint++;
            }

            OSUTF8Char *source = format - (charsToPrint + 2);
            SLPrintChars(source, charsToPrint);
            charsToPrint = 0;

            if (c == '%') continue;
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
                inPadding = true;
                padding = 0;
            } break;
            case 'h': {
                if (lastFlag == c) {
                    argSize = 1;
                } else {
                    lastFlag = c;
                    argSize = 2;
                }
            } break;
            case 'l': {
                if (lastFlag == c) {
                    argSize = 8;
                } else {
                    lastFlag = c;
                    argSize = 4;
                }
            } break;
            case 'z': {
                argSize = 8;
            } break;
            // Extension to print binary
            // numbers of length n
            case 'b': {
                inEscapeCode = false;
                printNumber = true;
                base = 2;
            } break;
            case 'o': {
                inEscapeCode = false;
                printNumber = true;
                base = 8;
            } break;
            case 'u': {
                inEscapeCode = false;
                printNumber = true;
                printSign = false;
                base = 10;
            } break;
            case 'd': {
                inEscapeCode = false;
                printNumber = true;
                printSign = true;
                base = 10;
            } break;
            case 'x': {
                inEscapeCode = false;
                printNumber = true;
                hexBase = 'a';
                base = 16;
            } break;
            case 'X': {
                inEscapeCode = false;
                printNumber = true;
                hexBase = 'A';
                base = 16;
            } break;
            case 'p': {
                SLPrintChars((OSUTF8Char *)"0x", 2);

                inEscapeCode = false;
                printNumber = true;
                hexBase = 'A';
                argSize = 8;
                base = 16;
            } break;
            case 'c': {
                UInt8 character = OSVAGetNext(args, UInt32);
                inEscapeCode = false;

                SLPrintChars(&character, 1);
            } break;
            case 's': {
                inEscapeCode = false;

                OSUTF8Char *utf8string = OSVAGetNext(args, OSUTF8Char *);
                SLPrintChars(utf8string, CXKStringSize8(utf8string));
            } break;
            case 'S': {
                inEscapeCode = false;

                OSUTF16Char *utf16string = OSVAGetNext(args, OSUTF16Char *);
                OSUTF8Char *utf8string = SLUTF16ToUTF8(utf16string);

                if (!utf8string)
                {
                    SLPrintChars((OSUTF8Char *)"<error>", 7);

                    break;
                }

                SLPrintChars(utf8string, CXKStringSize8(utf8string));
                SLFree(utf8string);
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
                inEscapeCode = false;
                hexDump = true;
            } break;
        }

        if (inPadding)
        {
            if ('0' <= c && c <= '9') {
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
                #define entry(l) {                  \
                    UInt ## l *arg = argument;      \
                    number = (*arg);                \
                    argument += sizeof(UInt ## l);  \
                } break

                switch (argSize)
                {
                    case 1: entry(8);
                    case 2: entry(16);
                    case 4: entry(32);
                    case 8: entry(64);
                }

                #undef entry

                SLNumberToStringConverter(number, false, 16, (argSize * 2), 'A', true, kOSNullPointer, kOSNullPointer);

                if (padding != 1)
                    SLPrintChars((OSUTF8Char *)" ", 1);

                padding--;
            }

            hexDump = false;
            lastFlag = 0;
            argSize = 4;
            padding = 1;
        }

        if (printNumber)
        {
            SInt64 number;

            if (!printSign) {
                switch (argSize)
                {
                    case 1:
                    case 2:
                    case 4:
                        number = OSVAGetNext(args, UInt32);
                    break;
                    case 8:
                        number = OSVAGetNext(args, UInt64);
                    break;
                    default: number = 0;
                }
            } else {
                switch (argSize)
                {
                    case 1:
                    case 2:
                    case 4:
                        number = OSVAGetNext(args, SInt32);
                    break;
                    case 8:
                        number = OSVAGetNext(args, SInt64);
                    break;
                    default: number = 0;
                }
            }

            SLNumberToStringConverter(number, printSign, base, padding, hexBase, true, kOSNullPointer, kOSNullPointer);

            printNumber = false;
            printSign = false;
            lastFlag = 0;
            padding = 1;
        }
    }

    if (charsToPrint)
    {
        OSUTF8Char *source = (format - charsToPrint);
        SLPrintChars(source, charsToPrint);
    }

    #undef kSLCopyArgs
}

#if !kCXTargetOSApple
    void SLPrintError(const char *s, ...) OSAlias(SLPrintString);
#endif /* !kCXTargetOSApple */
