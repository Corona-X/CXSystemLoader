#include <Kernel/Shared/XKAssemblyCode.h>

#if kCXArchIA

.section kXKCodeSectionName
.align kXKNaturalAlignment
.code64

// Arguments:
//
// Return Value (%rax):
//
// Destroyed Registers:
//   %rax
//   %rcx
//   %rdx
//
// If Debug:
//
XKDeclareFunction(SLSetupCPU):
    //cli
    //movq $0xC0000080, %rcx
    //rdmsr
    //andq $0xFFFF46FE, %rax
    //leaq _protected(%rip), %r8
    //wrmsr
    //movq %r8, %rax
    //jmpq *%rax

.code32
.align 4

_protected:
    //
    ret

.code16

#endif /* Architecture */
