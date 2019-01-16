#include <Kernel/XKAssemblyCode.h>

.section kXKCodeSectionName
.align   kXKNaturalAlignment

XKDeclareFunction(SLProcessorSupportsCPUID):
    movq $1, %rax
    ret
