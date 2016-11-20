#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLConfigFile.h>
#include <SystemLoader/SLFormattedPrint.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLoader.h>

OSAddress SLGetMainImageHandle(void)
{
    return gSLLoaderImageHandle;
}

SLRuntimeServices *SLRuntimeServicesGetCurrent(void)
{
    return gSLLoaderSystemTable->runtimeServices;
}

bool SLDelayProcessor(UIntN time, bool useBootServices)
{
    if (useBootServices && gSLBootServicesEnabled) {
        SLStatus status = SLBootServicesGetCurrent()->stall(time);
        return !SLStatusIsError(status);
    } else {
        // Try to mimic BS stall() function
        for (volatile UInt64 i = 0; i < (time * 100); i++);
        return true;
    }
}

char SLWaitForKeyPress(void)
{
    SLBootServicesCheck(0);

    SLStatus status = gSLLoaderSystemTable->stdin->reset(gSLLoaderSystemTable->stdin, false);
    if (SLStatusIsError(status)) return 0;
    status = kSLStatusNotReady;
    SLKeyPress key;
    
    while (status == kSLStatusNotReady)
        status = gSLLoaderSystemTable->stdin->readKey(gSLLoaderSystemTable->stdin, &key);
    
    return key.keycode;
}

#if kCXBuildDev
    bool SLPromptUser(const char *s, SLSerialPort port)
    {
        SLPrintString("%s (y/n)? ", s);
        UInt8 response = 0;

        if (!gSLBootServicesEnabled) {
            while (response != 'y' && response != 'n')
                response = SLSerialReadCharacter(port, true);
        } else {
            gSLLoaderSystemTable->stdin->reset(gSLLoaderSystemTable->stdin, false);
            SLKeyPress key; key.keycode = 0;
            SLStatus status;

            while ((key.keycode != 'y' && response != 'y') && (key.keycode != 'n' && response != 'n'))
            {
                status = gSLLoaderSystemTable->stdin->readKey(gSLLoaderSystemTable->stdin, &key);
                response = SLSerialReadCharacter(port, false);

                if (status != kSLStatusNotReady)
                    response = key.keycode;
            }

            if (key.keycode == 'y')
                response = 'y';

            if (key.keycode == 'n')
                response = 'n';
        }

        SLPrintString("%c\n", response);
        return (response == 'y');
    }

    void SLShowDelay(const char *s, UInt64 seconds)
    {
        SLPrintString("%s in ", s);

        while (seconds)
        {
            SLPrintString("%d...", seconds);
            SLDelayProcessor(1000000, gSLBootServicesEnabled);
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
            SLPrintString("efer: 0x%zX\n", efer);
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

    #define SLPrintRegister(s, r)   SLPrintString(OSStringValue(r) ": 0x%zX\n", s->r)
    #define SLPrintRegister16(s, r) SLPrintString(OSStringValue(r) ": 0x%hX\n", s->r)

    #define SLPrint2Registers(s, r0, r1)            \
        SLPrintRegister(s, r0);                     \
        SLPrintRegister(s, r1);

    #define SLPrint4Registers(s, r0, r1, r2, r3)    \
        SLPrint2Registers(s, r0, r1);               \
        SLPrint2Registers(s, r2, r3);

    #define SLPrint2Registers16(s, r0, r1)          \
        SLPrintRegister16(s, r0);                   \
        SLPrintRegister16(s, r1);

    #define SLPrint4Registers16(s, r0, r1, r2, r3)  \
        SLPrint2Registers16(s, r0, r1);             \
        SLPrint2Registers16(s, r2, r3);

    void SLPrintBasicState(XKProcessorBasicState *state)
    {
        SLPrint4Registers(state, rax, rbx, rcx, rdx);
        SLPrint4Registers(state, r8,  r9,  r10, r11);
        SLPrint4Registers(state, r12, r13, r14, r15);
        SLPrint4Registers(state, rsi, rdi, rbp, rsp);
        SLPrint2Registers(state, rip, rflags);
        SLPrint4Registers16(state, cs, ds, ss, es);
        SLPrint2Registers16(state, fs, gs);
    }

    void SLPrintSystemState(XKProcessorSystemState *state)
    {
        SLPrint4Registers(state, cr0, cr2, cr3, cr4);
        SLPrintRegister(state, cr8);

        SLPrintString("gdtr: 0x%X (limit = 0x%hX)\n", state->gdtr.base, state->gdtr.limit);
        SLPrintString("idtr: 0x%X (limit = 0x%hX)\n", state->idtr.base, state->idtr.limit);

        SLPrint2Registers16(state, ldtr, tr);
    }

    void SLPrintDebugState(XKProcessorDebugState *state)
    {
        SLPrint4Registers(state, dr0, dr1, dr2, dr3);
        SLPrint2Registers(state, dr6, dr7);
    }

    void __SLLibraryInitialize(void)
    {
        SLMemoryAllocatorInit();
        SLConfigGet();

        __SLSerialConsoleInitAll();
        __SLBitmapFontInitialize();
        __SLVideoConsoleInitAll();
    }

    void SLUnrecoverableError(void)
    {
        OSFault();
    }
#else /* !kCXBuildDev */
    void SLUnrecoverableError(void)
    {
        OSFault();
    }
#endif /* kCXBuildDev */
