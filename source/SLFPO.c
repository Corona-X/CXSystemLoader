#include <SystemLoader/SystemLoader.h>
#include <System/OSCompilerMacros.h>
#include <SystemLoader/SLLibrary.h>

typedef UInt32 OSUnicodePoint;

static const OSUnicodePoint kSLSurrogateHighBegin  = 0xD800;
static const OSUnicodePoint kSLSurrogateHighFinish = 0xDBFF;
static const OSUnicodePoint kSLSurrogateLowBegin   = 0xDC00;
static const OSUnicodePoint kSLSurrogateLowFinish  = 0xDFFF;
static const OSUnicodePoint kSLErrorCodePoint = 0xFFFFFFFF;

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

OSPrivate OSCount SLUTF8FromCodePoint(OSUnicodePoint point, OSUTF8Char *output, OSCount count);
OSPrivate OSUnicodePoint SLUTF8ToCodePoint(OSUTF8Char *input, OSCount inCount, OSCount *usedCount);
OSPrivate OSCount SLUTF16FromCodePoint(OSUnicodePoint point, OSUTF16Char *output, OSCount count);

OSPrivate OSUnicodePoint SLUTF16ToCodePoint(OSUTF16Char *input, OSCount inCount, OSCount *usedCount);

static OSLength SLSimpleStringLengthUTF8(OSUTF8Char *string)
{
    OSLength length = 0;
    
    while (*string)
    {
        string++;
        length++;
    }
    
    return length;
}

static OSLength SLSimpleStringLengthUTF16(OSUTF16Char *string)
{
    OSLength length = 0;
    
    while (*string)
    {
        string++;
        length++;
    }
    
    return length;
}

void SLLoaderSerial0OutputUTF8(UInt8 character)
{
    SLSerialPort port0 = 0x03F8;

    SLSerialWriteCharacter(port0, character, true);
}

OSInline void SLPrintSingle(UInt8 character)
{
    SLLoaderSerial0OutputUTF8(character);
}

OSInline void SLPrintUTF8Single(OSUTF8Char character[7])
{
    UInt8 n = 0;

    while (character[n])
    {
        SLLoaderSerial0OutputUTF8(character[n]);
        n++;
    }
}

void SLPN(UInt64 number, UInt8 base, UInt8 hexBegin, UInt8 padding)
{
    OSUTF8Char  buffer[32];
    SInt8 i = 0;
    
    if (!number)
    {
        if (padding)
            for ( ; i < padding; i++)
                SLPrintSingle('0');
        
        return;
    }

    do {
        UInt8 next = number % base;
        number /= base;
        
        if (next < 10) {
            buffer[i++] = next + '0';
        } else if (next < 37) {
            buffer[i++] = (next - 10) + hexBegin;
        } else {
            buffer[i++] = '#';
        }
    } while (number);
    
    if (i < padding)
    {
        UInt8 zeroCount = padding - i;
        
        for (UInt8 j = 0; j < zeroCount; j++)
            SLPrintSingle('0');
    }
    
    for (i--; i >= 0; i--)
        SLPrintSingle(buffer[i]);
}

void SLPS(const char *s, ...)
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
    
    OSVAList args;
    OSVAStart(args, s);
    
    while (*s)
    {
        UInt8 c = *s++;
        
        if (!inEscapeCode && c != '%')
        {
            SLPrintSingle(c);
            
            continue;
        }
        
        if (!inEscapeCode)
        {
            justEnteredEscapeCode = true;
            inEscapeCode = true;
            
            continue;
        }
        
        if (justEnteredEscapeCode)
        {
            justEnteredEscapeCode = false;
            
            if (c == '%')
            {
                inEscapeCode = false;
                SLPrintSingle(c);
                
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
                SLPrintSingle('0');
                SLPrintSingle('x');
                
                inEscapeCode = false;
                printNumber = true;
                hexBase = 'A';
                argSize = 8;
                base = 16;
            } break;
            case 'c': {
                UInt8 character = OSVAGetNext(args, UInt32);
                inEscapeCode = false;
                
                SLPrintSingle(character);
            } break;
            case 's': {
                inEscapeCode = false;
                OSUTF8Char *utf8string = OSVAGetNext(args, OSUTF8Char *);
                OSCount charsLeft = SLSimpleStringLengthUTF8(utf8string);
                
                while (charsLeft)
                {
                    SLPrintSingle(*utf8string++);
                    charsLeft--;
                }
            } break;
            case 'S': {
                inEscapeCode = false;
                OSUTF16Char *utf16string = OSVAGetNext(args, OSUTF16Char *);
                OSCount charsLeft = SLSimpleStringLengthUTF16(utf16string);
                OSCount used, used8;
                OSUTF8Char utf8[7];

                while (charsLeft)
                {
                    OSUnicodePoint point = SLUTF16ToCodePoint(utf16string, charsLeft, &used);
                    if (point == kSLErrorCodePoint) break;
                    used++;
                    
                    used8 = SLUTF8FromCodePoint(point, utf8, 6);
                    utf8[used8 + 1] = 0;
                    
                    utf16string += used;
                    charsLeft -= used;
                    
                    SLPrintUTF8Single(utf8);
                }
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
                
                SLPN(number, 16, 'A', argSize * 2);
                if (padding != 1) SLPrintSingle(' ');
                padding--;
            }
            
            hexDump = false;
            argSize = 4;
            padding = 1;
        }
        
        if (printNumber)
        {
            UInt64 number;
            
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
                SInt64 signedValue;
                
                switch (argSize)
                {
                    case 1:
                    case 2:
                    case 4:
                        signedValue = OSVAGetNext(args, SInt32);
                        break;
                    case 8:
                        signedValue = OSVAGetNext(args, SInt64);
                        break;
                    default: signedValue = 0;
                }
                
                if (signedValue < 0)
                {
                    signedValue *= -1;
                    
                    SLPrintSingle('-');
                }
                
                printSign = false;
                number = signedValue;
            }
            
            SLPN(number, base, hexBase, padding);
            printNumber = false;
            padding = 1;
        }
    }
    
    OSVAFinish(args);
}
