/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBase.h - Base Types and Structures for EFI Development        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBASE__
#define __SYSTEMLOADER_SLBASE__ 1

#include <Corona-X.h>
#include <System/OSTypes.h>
#include <System/OSUID.h>
#include <System/OSCompilerMacros.h>
#include <System/OSGPT.h>
#include <System/OSByteMacros.h>
#include <Kernel/CXKBootOptions.h>
#include <Kernel/CXKPOST.h>
#include <Kernel/CXKMemoryIO.h>
#include <Kernel/CXKProcessorState.h>
#include <Kernel/CXKMemory.h>

#define kSLACPITableID                  ((SLProtocol){0x8868E871, 0xE4F1, 0x11D3, {0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}})
#define kSLGraphicsOutputProtocol       ((SLProtocol){0x9042A9DE, 0x23DC, 0x4A38, {0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A}})
//#define kSLGraphicsOutputProtocol       ((SLProtocol){0x982C298B, 0xF4FA, 0x41CB, {0xB8, 0x38, 0x77, 0xAA, 0x68, 0x8F, 0xB8, 0x39}})
#define kSLLoadedImageProtocol          ((SLProtocol){0x5B1B31A1, 0x9562, 0x11D2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}})
#define kSLVolumeProtocol               ((SLProtocol){0x964E5B22, 0x6459, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}})
#define kSLFileInfoID                   ((SLProtocol){0x09576E92, 0x6D3F, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}})

#define kSLStatusIncompatibleVersion    ((SLStatus)(25 | (1ULL << 63ULL)))
#define kSLStatusWrongSize              ((SLStatus)(4  | (1ULL << 63ULL)))
#define kSLStatusBufferTooSmall         ((SLStatus)(5  | (1ULL << 63ULL)))
#define kSLStatusNotReady               ((SLStatus)(6  | (1ULL << 63ULL)))
#define kSLStatusLoadError              ((SLStatus)(1  | (1ULL << 63ULL)))
#define kSLStatusSuccess                0

#define SLABI                           __attribute__((ms_abi))
#define SLPrivate                       OSPrivate SLABI
#define kSLBootPageSize                 4096

#define SLStatusIsError(s)              ((s >> 63) & 1)

typedef OSUIDIntelData SLProtocol;
typedef UInt16 SLSerialPort;
typedef UIntN SLStatus;

typedef struct {
    UInt64 signature;
    UInt32 revision;
    UInt32 headerSize;
    UInt32 checksum;
    UInt32 reserved;
} SLTableHeader;

#endif /* !defined(__SYSTEMLOADER_SLBASE__) */
