#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLConfigFile.h>
#include <SystemLoader/SLFormattedPrint.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLoader.h>
#include <SystemLoader/EFI/SLSystemTable.h>
#include <Kernel/XKMemory.h>

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

char SLWaitForKeyPress(void)
{
    if (!SLBootServicesHaveTerminated()) {
        SLStatus status;
        SLKeyPress key;

        do {
            status = SLSystemTableGetCurrent()->stdin->readKey(gSLLoaderSystemTable->stdin, &key);
        } while (status == kSLStatusNotReady);

        return key.keycode;
    } else {
        // Need USB Keyboard Driver at this Point...
        return 0;
    }
}

#if kCXBuildDev
    bool SLPromptUser(const char *s)
    {
        SLPrintString("%s (yes/no)? ", s);
        UInt8 response = 0;

        for ( ; ; )
        {
            bool result = false, again = true;
            OSSize size;

            OSUTF8Char *string = SLScanString('\r', &size);
            if (size == 0) continue;
            SLPrintString("\n");

            if (!XKMemoryCompare(string, "yes", 3)) {
                result = true;
                again = false;
            } else if (!XKMemoryCompare(string, "no", 2)) {
                result = false;
                again = false;
            }

            SLFree(string);

            if (again)
            {
                SLPrintString("Invalid Response. %s (yes/no)? ", s);
                continue;
            }

            return result;
        }
    }

    void SLShowDelay(const char *s, UInt64 seconds)
    {
        SLPrintString("%s in ", s);

        while (seconds)
        {
            SLPrintString("%d...", seconds);
            SLDelayProcessor(1000000, true);
            SLDeleteCharacters(4);

            seconds--;
        }

        SLDeleteCharacters(3);
        SLPrintString("Now.\n");
    }

    void SLDumpProcessorState(bool standard, bool system, bool debug)
    {
        if (standard)
        {
            XKProcessorBasicState state;

            XKProcessorGetBasicState(&state);
            SLPrintString("Basic Register State:\n");
            SLPrintBasicState(&state);
            SLPrintString("\n");
        }

        if (system)
        {
            XKProcessorSystemState state;

            XKProcessorGetSystemState(&state);
            SLPrintString("System Register State:\n");
            SLPrintSystemState(&state);

            XKProcessorMSR efer = XKProcessorMSRRead(0xC0000080);
            SLPrintString("efer: 0x%016zX\n", efer);
            SLPrintString("\n");
        }

        if (debug)
        {
            XKProcessorDebugState state;

            XKProcessorGetDebugState(&state);
            SLPrintString("Debug Register State:\n");
            SLPrintDebugState(&state);
            SLPrintString("\n");
        }
    }

    #define SLPrintRegister(s, r, l, e)   SLPrintString(OSStringValue(r) e ": 0x%016" l "X", s->r)

    #define SLPrint2Registers(s, r0, r1, l, e)      \
        SLPrintRegister(s, r0, l, e);               \
        SLPrintString(", ");                        \
        SLPrintRegister(s, r1, l, e)

    #define SLPrint2RegistersS(s, r0, r1, l, e)     \
        SLPrint2Registers(s, r0, r1, l, e);         \
        SLPrintString(", ")

    #define SLPrint2RegistersE(s, r0, r1, l, e)     \
        SLPrint2Registers(s, r0, r1, l, e);         \
        SLPrintString("\n")

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
        SLPrintString("\n");

        SLPrintString("gdtr: 0x%016zX (limit = 0x%04hX)\n", state->gdtr.base, state->gdtr.limit);
        SLPrintString("idtr: 0x%016zX (limit = 0x%04hX)\n", state->idtr.base, state->idtr.limit);
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
        SLSystemTable *systemTable = SLSystemTableGetCurrent();
        systemTable->stdin->reset(systemTable->stdin, false);

        SLMemoryAllocatorInit();
        SLConfigGet();

        __SLSerialConsoleInitAll();
        __SLBitmapFontInitialize();
        __SLVideoConsoleInitAll();
    }

    void SLUnrecoverableError(void)
    {
        SLPrintError("Unrecoverable Error.\n");
        OSFault();
    }
#else /* !kCXBuildDev */
    void SLUnrecoverableError(void)
    {
        SLPrintError("Unrecoverable Error.\n");
        OSFault();
    }
#endif /* kCXBuildDev */
