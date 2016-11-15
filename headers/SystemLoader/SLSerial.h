/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLSerial.h - System Loader Serial Driver                        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLSERIAL__
#define __SYSTEMLOADER_SLSERIAL__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if kCXBuildDev
    #define kSLSerialAssumedClockSpeed 1843200
    #define kSLSerialPortError         0xFFFF

    #define kSLSerialWordLength5Bits    0x0
    #define kSLSerialWordLength6Bits    0x1
    #define kSLSerialWordLength7Bits    0x2
    #define kSLSerialWordLength8Bits    0x3

    #define kSLSerial1StopBit           0x0
    #define kSLSerial2StopBits          0x1

    #define kSLSerialNoParity           0x0
    #define kSLSerialOddParity          0x1
    #define kSLSerialEvenParity         0x3
    #define kSLSerialMarkParity         0x5
    #define kSLSerialSpaceParity        0x7

    #if kCXBootloaderCode
        SLPrivate SLSerialPort SLSerialPortInit(OSAddress portBase);
        SLPrivate void SLSerialPortReset(SLSerialPort port);
        SLPrivate void SLSerialPortSetupLineControl(SLSerialPort port, UInt8 size, UInt8 parity, UInt8 stop);
        SLPrivate void SLSerialPortSetBaudDivisor(SLSerialPort port, UInt16 divisor);
        SLPrivate void SLSerialPortSetBaudRate(SLSerialPort port, UInt32 rate);

        SLPrivate void SLSerialWriteCharacter(SLSerialPort port, UInt8 character, bool block);
        SLPrivate UInt8 SLSerialReadCharacter(SLSerialPort port, bool block);

        SLPrivate void SLSerialWriteString(SLSerialPort port, UInt8 *string);
        SLPrivate void SLSerialReadString(SLSerialPort port, UInt8 terminator, OSBuffer *buffer, bool print);
    #endif /* kCXBootloaderCode */
#endif /* kCXBuildDev */

#endif /* !defined(__SYSTEMLOADER_SLSERIAL__) */
