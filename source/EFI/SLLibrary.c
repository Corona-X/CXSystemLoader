#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLConfigFile.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLoader.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <Kernel/XKShared.h>

OSAddress SLGetMainImageHandle(void)
{
    return gSLLoaderImageHandle;
}

SLRuntimeServices *SLRuntimeServicesGetCurrent(void)
{
    return SLSystemTableGetCurrent()->runtimeServices;
}

bool SLDelayProcessor(UIntN time, bool useBootServices)
{
    if (useBootServices && !SLBootServicesHaveTerminated()) {
        SLStatus status = SLBootServicesGetCurrent()->stall(time);
        return !SLStatusIsError(status);
    } else {
        // Try to mimic BS stall() function
        // This only works before MP has been enabled,
        // and the timing is very inacurate....
        for (volatile UInt64 i = 0; i < (time * 100); i++);
        return true;
    }
}

#if kCXBuildDev
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
            SLPrintBasicState(&state);
            XKPrintString("\n");
        }

        if (system)
        {
            XKProcessorSystemState state;

            XKProcessorGetSystemState(&state);
            XKLog(kXKLogLevelVerbose, "System Register State:\n");
            SLPrintSystemState(&state);

            XKProcessorMSR efer = XKProcessorMSRRead(0xC0000080);
            XKLog(kXKLogLevelVerbose, "efer: 0x%016zX\n", efer);
            XKPrintString("\n");
        }

        if (debug)
        {
            XKProcessorDebugState state;

            XKProcessorGetDebugState(&state);
            XKLog(kXKLogLevelVerbose, "Debug Register State:\n");
            SLPrintDebugState(&state);
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

    void SLPrintBasicState(XKProcessorBasicState *state)
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

    void SLPrintSystemState(XKProcessorSystemState *state)
    {
        SLPrint2RegistersS(state, cr0, cr2, "z", "");
        SLPrint2RegistersE(state, cr3, cr4, "z", "");
        SLPrintRegister(state, cr8, "z", "");
        XKPrintString("\n");

        XKPrintString("gdtr: 0x%016zX (limit = 0x%04hX)\n", state->gdtr.base, state->gdtr.limit);
        XKPrintString("idtr: 0x%016zX (limit = 0x%04hX)\n", state->idtr.base, state->idtr.limit);
        SLPrint2RegistersE(state, ldtr, tr, "h", "");
    }

    void SLPrintDebugState(XKProcessorDebugState *state)
    {
        SLPrint2RegistersS(state, dr0, dr1, "z", "");
        SLPrint2RegistersE(state, dr2, dr3, "z", "");
        SLPrint2RegistersE(state, dr6, dr7, "z", "");
    }

    void __SLLibraryInitialize(void)
    {
        SLMemoryAllocatorInit();
        SLConfigGet();

        __XKInputConsoleInitAllEFI();
        __XKSerialConsoleInitAll();
        __XKBitmapFontInitialize();
        __XKVideoConsoleInitAll();
    }

    void SLUnrecoverableError(void)
    {
        XKLog(kXKLogLevelError, "Unrecoverable Error.\n");
        OSFault();
    }
#else /* !kCXBuildDev */
    void SLUnrecoverableError(void)
    {
        XKLog(kXKLogLevelError, "Unrecoverable Error.\n");
        OSFault();
    }
#endif /* kCXBuildDev */
