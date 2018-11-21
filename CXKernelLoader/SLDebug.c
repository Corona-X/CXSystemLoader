#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLGraphics.h>
#include <SystemLoader/Kernel/SLDebug.h>

#if 0

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

void SLSystemTableDumpConfigTables(SLSystemTable *table)
{
    UIntN count = table->numberOfConfigTables;

    if (!count)
    {
        XKLog(kXKLogLevelVerbose, "Warning: No UEFI Config Tables Found!\n");
        return;
    }

    XKLog(kXKLogLevelVerbose, "Dumping %d UEFI Configuration Tables:\n", count);
    XKLog(kXKLogLevelVerbose, "First table is at %p\n", table->configTables);

    for (OSIndex i = 0; i < count; i++)
    {
        SLConfigTable *configTable = &table->configTables[i];
        OSUTF8Char *id = XKUIDToString(&configTable->id);

        XKPrintString("Config Table %d: {\n", i);
        XKLog(kXKLogLevelVerbose, "    ID: %s\n", id);
        XKLog(kXKLogLevelVerbose, "    Address: %p\n", configTable->pointer);
        XKLog(kXKLogLevelVerbose, "} ");

        SLFree(id);
    }

    XKPrintString("\n\n");
}

void SLDumpConsoles(void)
{
    XKLog(kXKLogLevelDebug, "Consoles:\n");
    XKConsole *console = gXKConsoleFirst;

    while (console)
    {
        XKLog(kXKLogLevelDebug, "%u: %p\n", console->id, console);
        XKLog(kXKLogLevelDebug, "  --> context: %p\n", console->context);
        XKLog(kXKLogLevelDebug, "  --> output:  %p\n", console->write);
        XKLog(kXKLogLevelDebug, "  --> level:   %d\n", console->level);
        XKLog(kXKLogLevelDebug, "  --> next:    %p\n", console->next);

        console = console->next;
    }
}

bool SLPromptUser(const char *s)
{
    XKLog(kXKLogLevelInfo, "%s (yes/no)? ", s);
    UInt8 response = 0;

    for ( ; ; )
    {
        bool result = false, again = true;
        OSSize size;

        OSUTF8Char *string = XKConsoleReadString('\n', &size);
        if (!string) continue;

        if ((size == 3) && !XKMemoryCompare(string, "yes", 3)) {
            result = true;
            again = false;
        } else if ((size == 2) && !XKMemoryCompare(string, "no", 2)) {
            result = false;
            again = false;
        }

        SLFree(string);

        if (again)
        {
            XKLog(kXKLogLevelInfo, "Invalid Response. %s (yes/no)? ", s);
            continue;
        }

        return result;
    }
}

void SLShowDelay(const char *s, UInt64 seconds)
{
    XKLog(kXKLogLevelInfo, "%s in ", s);

    while (seconds)
    {
        XKPrintString("%d...", seconds);
        SLDelayProcessor(1000000, true);
        XKConsoleDeleteCharacters(4);

        seconds--;
    }

    XKConsoleDeleteCharacters(3);
    XKPrintString("Now.\n");
}

void SLDumpProcessorState(bool standard, bool system, bool debug)
{
    if (standard)
    {
        XKProcessorBasicState state;

        XKProcessorGetBasicState(&state);
        XKLog(kXKLogLevelVerbose, "Basic Register State:\n");
        SLDumpBasicState(&state);
        XKPrintString("\n");
    }

    if (system)
    {
        XKProcessorSystemState state;

        XKProcessorGetSystemState(&state);
        XKLog(kXKLogLevelVerbose, "System Register State:\n");
        SLDumpSystemState(&state);

        XKProcessorMSR efer = XKProcessorMSRRead(0xC0000080);
        XKLog(kXKLogLevelVerbose, "efer: 0x%016zX\n", efer);
        XKPrintString("\n");
    }

    if (debug)
    {
        XKProcessorDebugState state;

        XKProcessorGetDebugState(&state);
        XKLog(kXKLogLevelVerbose, "Debug Register State:\n");
        SLDumpDebugState(&state);
        XKPrintString("\n");
    }
}

#define SLPrintRegister(s, r, l, e)   XKLog(kXKLogLevelVerbose, OSStringValue(r) e ": 0x%016" l "X", s->r)

#define SLPrint2Registers(s, r0, r1, l, e)      \
SLPrintRegister(s, r0, l, e);               \
XKPrintString(", ");                        \
SLPrintRegister(s, r1, l, e)

#define SLPrint2RegistersS(s, r0, r1, l, e)     \
SLPrint2Registers(s, r0, r1, l, e);         \
XKPrintString(", ")

#define SLPrint2RegistersE(s, r0, r1, l, e)     \
SLPrint2Registers(s, r0, r1, l, e);         \
XKPrintString("\n")

void SLDumpBasicState(XKProcessorBasicState *state)
{
    SLPrint2RegistersS(state, rax, rbx, "z", "");
    SLPrint2RegistersE(state, rcx, rdx, "z", "");
    SLPrint2RegistersS(state, r8,  r9,  "z", " ");
    SLPrint2RegistersE(state, r10, r11, "z", "");
    SLPrint2RegistersS(state, r12, r13, "z", "");
    SLPrint2RegistersE(state, r14, r15, "z", "");
    SLPrint2RegistersS(state, rsi, rdi, "z", "");
    SLPrint2RegistersE(state, rbp, rsp, "z", "");
    SLPrint2RegistersS(state, cs,  ds,  "h", " ");
    SLPrint2RegistersE(state, ss,  es,  "h", " ");
    SLPrint2RegistersE(state, fs,  gs,  "h", " ");
    SLPrint2RegistersE(state, rip, rflags, "z", "");
}

void SLDumpSystemState(XKProcessorSystemState *state)
{
    SLPrint2RegistersS(state, cr0, cr2, "z", "");
    SLPrint2RegistersE(state, cr3, cr4, "z", "");
    SLPrintRegister(state, cr8, "z", "");
    XKPrintString("\n");

    XKPrintString("gdtr: 0x%016zX (limit = 0x%04hX)\n", state->gdtr.base, state->gdtr.limit);
    XKPrintString("idtr: 0x%016zX (limit = 0x%04hX)\n", state->idtr.base, state->idtr.limit);
    SLPrint2RegistersE(state, ldtr, tr, "h", "");
}

void SLDumpDebugState(XKProcessorDebugState *state)
{
    SLPrint2RegistersS(state, dr0, dr1, "z", "");
    SLPrint2RegistersE(state, dr2, dr3, "z", "");
    SLPrint2RegistersE(state, dr6, dr7, "z", "");
}

void __SLLibraryInitialize(void)
{
    SLMemoryAllocatorInit();
    SLConfigGet();

    __XKBitmapFontInitialize();
    __XKInitializeLogging();

    XKLog(kXKLogLevelInfo, kXKLogSubsystemLoader "Kernel Logging Subsystem is now Online.");
}

#endif
