#include <Kernel/XKAssemblyCode.h>

.section kXKCodeSectionName

XKDeclareFunction(SLProcessorSupportsCPUID):
    pushfq                          // save original flags

    pushfq                          // push flags to stack
    xorq $0x00200000, (%rsp)        // flip id bit of flags copy on stack

    popfq                           // restore with flipped flags bit
    pushfq                          // save with flipped bit

    popq %rax                       // load accumulator with modified flags
    xorq (%rsp), %rax               // get rid of matching flags
    shrq $21, %rax                  // rax = 1 if id bit has been modified, else 0

    popfq                           // restore original flags register
    ret                             // leave
