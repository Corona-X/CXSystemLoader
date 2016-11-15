/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBitmapFont.h - Bitmap fonts for System Loader                 */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 31.10.2016 - 11:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 31.10.2016 - 11:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBITMAPFONT__
#define __SYSTEMLOADER_SLBITMAPFONT__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

typedef struct {
    UInt32 height;
    UInt32 width;
    UInt8 *packedData;
    UInt32 *fontData;
} SLBitmapFont;

OSExport SLBitmapFont gSLBitmapFont8x16;

#endif /* !defined(__SYSTEMLOADER_SLBITMAPFONT__) */
