#include <Kernel/XKAssembly.h>

#if kCXArchIA

.section kXKCodeSectionName
.align kXKNaturalAlignment

// Arguments:
//   %rcx: Image Handle
//   %rdx: System Table
//
// Return Value (%rax):
//   0: success
//   other: failure
//
// Destroyed Registers:
//   %rax
//   %rbx
//   %rcx
//   %rdx
//   %rdi
//   %rsi
//
// If Debug:
//
XKDeclareFunction(_SLEntry):
    movq (%rsp), %rax

    XKLoadSymbol(gSLFirmwareReturnAddress, %rbx)
    movq %rax, (%rbx)

    XKLoadSymbol(gSLLoaderImageHandle, %rbx)
    movq %rcx, (%rbx)

    XKLoadSymbol(gSLLoaderSystemTable, %rbx)
    movq %rdx, (%rbx)

    #if kCXBuildDev && !kCXTargetOSApple
        leaq gSLLoaderBase(%rip), %rbx
        leaq gSLLoaderEnd(%rip), %rax
        subq %rbx, %rax
        XKLoadSymbol(gSLLoaderImageSize, %rbx)
        movq %rax, (%rbx)

        XKLoadSymbol(gSLEnableScreenPrint, %rbx)
        movb $1, (%rbx)
    #endif /* kCXBuildDev */

    XKLoadSymbol(gSLBootServicesEnabled, %rbx)
    movb $1, (%rbx)

    #if kCXBuildDev
        leaq XKSymbol(__SLLibraryInitialize)(%rip), %rax
        callq *%rax
    #else /* !kCXBuildDev */
        leaq XKSymbol(SLMemoryAllocatorInit)(%rip), %rax
        callq *%rax
    #endif /* kCXBuildDev */

    XKLoadSymbol(gSLLoaderImageHandle, %rbx)
    movq (%rbx), %rdi

    XKLoadSymbol(gSLLoaderSystemTable, %rbx)
    movq (%rbx), %rsi

    leaq XKSymbol(CXSystemLoaderMain)(%rip), %rax
    callq *%rax

    movq %rax, %rcx
    leaq XKSymbol(SLLeave)(%rip), %rax
    callq *%rax

.align kXKNaturalAlignment

// Arguments:
//   %rcx: Exit Code
//
// This function will exit the loader
// without returning. It can do pretty
// much whatever it wants...
XKDeclareFunction(SLLeave):
    movq %rcx, %rdx
    XKLoadSymbol(gSLLoaderImageHandle, %rbx)
    movq (%rbx), %rcx
    xorq %r8, %r8
    xorq %r9, %r9
    XKLoadSymbol(gSLLoaderSystemTable, %rax)
    movq (%rax), %rbx
    movq 0x60(%rbx), %rax
    callq *0xD8(%rax)

    // If we're here, there was an error exiting.
    // Return to the address given by the firmware
    // on the initial program stack (saved in entry)
    XKLoadSymbol(gSLFirmwareReturnAddress, %rbx)
    movq (%rbx), %rax // Load return address into accumulator
    popq %rbx         // Kill real return address on the stack
    pushq (%rax)      // Inject firmware return address
    ret               // Return to given address

.comm XKSymbol(gSLFirmwareReturnAddress), 8, 8
.comm XKSymbol(gSLLoaderSystemTable),     8, 8
.comm XKSymbol(gSLLoaderImageHandle),     8, 8
.comm XKSymbol(gSLBootServicesEnabled),   1, 1

#if kCXBuildDev
    .comm XKSymbol(gSLLoaderImageSize),   8, 8
    .comm XKSymbol(gSLEnableScreenPrint), 1, 1
#endif /* kCXBuildDev */

#if kCXTargetOSLinux
.section .reloc, "a", @progbits

// Pretend like we're a relocatable executable...
.int 0
.int 10
.word 0
#endif /* Target OS */

#endif /* Arch */
