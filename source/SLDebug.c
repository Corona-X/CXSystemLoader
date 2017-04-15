#include <SystemLoader/EFI/SLGraphics.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <Kernel/XKShared.h>

bool SLGraphicsOutputDumpInfo(void)
{
    SLBootServicesCheck(false);
    
    OSCount count;
    SLGraphicsOutput **screens = SLGraphicsOutputGetAll(&count);
    
    if (!screens)
    {
        XKLog(kXKLogLevelVerbose, "Error: Could not enumerate connected screens\n");
        return false;
    }
    
    XKLog(kXKLogLevelVerbose, "Detected %zu connected screens:\n", count);
    
    for (OSCount i = 0; i < count; i++)
    {
        UInt32 modeCount = screens[i]->mode->numberOfModes;
        
        XKLog(kXKLogLevelVerbose, "screens[%u]: {\n", i);
        XKLog(kXKLogLevelVerbose, "    Address:          %p\n", screens[i]);
        XKLog(kXKLogLevelVerbose, "    Current Mode:     %u\n", screens[i]->mode->currentMode);
        XKLog(kXKLogLevelVerbose, "    Framebuffer Size: %zu\n", screens[i]->mode->framebufferSize);
        XKLog(kXKLogLevelVerbose, "    Framebuffer:      %p\n", screens[i]->mode->framebuffer);
        XKLog(kXKLogLevelVerbose, "    Modes (%u):\n", modeCount);
        
        for (UInt32 j = 0; j < modeCount; j++)
        {
            SLGraphicsModeInfo *mode = SLGraphicsOutputGetMode(screens[i], j);
            const char *format;
            
            if (!mode)
            {
                XKPrintString("Error: Could not get mode %d\n", j);
                return false;
            }
            
            switch (mode->format)
            {
                case kSLGraphicsPixelFormatRGBX8:   format = "RGB";      break;
                case kSLGraphicsPixelFormatBGRX8:   format = "BGR";      break;
                case kSLGraphicsPixelFormatBitMask: format = "Bit Mask"; break;
                case kSLGraphicsPixelFormatBLT:     format = "BLT";      break;
            }
            
            XKLog(kXKLogLevelVerbose, "        %03u: ", j);
            XKPrintString("{%p: %04ux%04u ", mode, mode->width, mode->height);
            XKPrintString("[%s; v%u;  ", format, mode->version);
            XKPrintString("%04u PPS]}\n", mode->pixelsPerScanline);
        }
        
        XKLog(kXKLogLevelVerbose, "}, ");
    }
    
    XKPrintString("\b\b  \b\b");
    XKPrintString("\n");
    
    return true;
}
