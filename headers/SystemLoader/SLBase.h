/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBase.h - Base Types and Structures for EFI Development        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 11:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBASE__
#define __SYSTEMLOADER_SLBASE__ 1

#include <Corona-X.h>
#include <System/OSTypes.h>
#include <System/OSUID.h>
#include <System/OSCompilerMacros.h>
#include <Kernel/Shared/XKLog.h>

#define kSLACPITableID                  ((SLProtocol){0x8868E871, 0xE4F1, 0x11D3, {0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}})
#define kSLGraphicsOutputProtocol       ((SLProtocol){0x9042A9DE, 0x23DC, 0x4A38, {0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A}})
#define kSLLoadedImageProtocol          ((SLProtocol){0x5B1B31A1, 0x9562, 0x11D2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}})
#define kSLVolumeProtocol               ((SLProtocol){0x964E5B22, 0x6459, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}})
#define kSLFileInfoID                   ((SLProtocol){0x09576E92, 0x6D3F, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}})

#define kSLStatusIncompatibleVersion    ((SLStatus)(25 | (1UL << 63UL)))
#define kSLStatusWrongSize              ((SLStatus)(4  | (1UL << 63UL)))
#define kSLStatusBufferTooSmall         ((SLStatus)(5  | (1UL << 63UL)))
#define kSLStatusNotReady               ((SLStatus)(6  | (1UL << 63UL)))
#define kSLStatusBadArgument            ((SLStatus)(2  | (1UL << 63UL)))
#define kSLStatusLoadError              ((SLStatus)(1  | (1UL << 63UL)))
#define kSLStatusSuccess                0

#define SLABI                           __attribute__((ms_abi))
#define kSLBootPageSize                 4096

#if !kCXAssemblyCode

#if !kCXBuildDev
    #define SLStatusIsError(s)          ((s >> 63) & 1)
#else /* kCXBuildDev */
    #define SLStatusIsError(s)                                  \
        ({                                                      \
            if (((s >> 63) & 1))                                \
                XKLog(kXKLogLevelWarning,                       \
                    "Status Error in function '%s' "            \
                    "in file '%s' on line %d."                  \
                    " (Status = 0x%zX)\n", __func__,            \
                    __FILE__, __LINE__, s);                     \
                                                                \
            ((s >> 63) & 1);                                    \
        })
#endif /* kCXBuildDev */

typedef OSUIDIntelData SLProtocol;
typedef UIntN SLStatus;

typedef struct {
    UInt64 signature;
    UInt32 revision;
    UInt32 headerSize;
    UInt32 checksum;
    UInt32 reserved;
} SLTableHeader;

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLBASE__) */
