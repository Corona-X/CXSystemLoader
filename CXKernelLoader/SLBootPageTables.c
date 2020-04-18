#include <System/OSTypes.h>

// These are the declarations of the page tables used on boot.
// These declarations would be places in SLProcessor.s, but they're much easier to write in C.
// Effectively, we identity map everything in 2 MB pages and also map them high in the last entries of PML4.

// 2 MB pages are 0x200000 bytes in length. The offset of each page is (n << 21)
#define SLDeclareBootPDE(offset, base) [(offset - base)] = ((((UInt64)(offset)) << 21) | (0x0000000000000083)),

// Macros used with the above to declare 512 entries which fill a PDE
#define __SLDo512(f, o, b) __SLDo256(f, o + 0, b) __SLDo256(f, o + 256, b)
#define __SLDo256(f, o, b) __SLDo128(f, o + 0, b) __SLDo128(f, o + 128, b)
#define __SLDo128(f, o, b) __SLDo64 (f, o + 0, b) __SLDo64 (f, o +  64, b)
#define __SLDo64(f, o, b)  __SLDo32 (f, o + 0, b) __SLDo32 (f, o +  32, b)
#define __SLDo32(f, o, b)  __SLDo16 (f, o + 0, b) __SLDo16 (f, o +  16, b)
#define __SLDo16(f, o, b)  __SLDo8  (f, o + 0, b) __SLDo8  (f, o +   8, b)
#define __SLDo8(f, o, b)   __SLDo4  (f, o + 0, b) __SLDo4  (f, o +   4, b)
#define __SLDo4(f, o, b)   __SLDo2  (f, o + 0, b) __SLDo2  (f, o +   2, b)
#define __SLDo2(f, o, b)   __SLDo   (f, o + 0, b) __SLDo   (f, o +   1, b)
#define __SLDo(f, o, b)    f(o, b)

const UInt64 _SLBootPML4[512] __attribute__((section("__BOOT,__pt"))) __attribute__((aligned(0x1000))) = {
    [0]   = 0x0000000000001003, // PDPT
    [511] = 0x0000000000001003  // PDPT
};

// We identity map the low 4 GB of RAM with these 4 PDEs below
const UInt64 _SLBootPDPT[512] __attribute__((section("__BOOT,__pt"))) = {
    [0] = 0x0000000000002003, // PDE0
    [1] = 0x0000000000003003, // PDE1
    [2] = 0x0000000000004003, // PDE2
    [3] = 0x0000000000005003  // PDE3
};

const UInt64 _SLBootPDE0[512] __attribute__((section("__BOOT,__pt"))) = {
    __SLDo512(SLDeclareBootPDE, 0x000, 0x000)
};

const UInt64 _SLBootPDE1[512] __attribute__((section("__BOOT,__pt"))) = {
    __SLDo512(SLDeclareBootPDE, 0x200, 0x200)
};

const UInt64 _SLBootPDE2[512] __attribute__((section("__BOOT,__pt"))) = {
    __SLDo512(SLDeclareBootPDE, 0x400, 0x400)
};

const UInt64 _SLBootPDE3[512] __attribute__((section("__BOOT,__pt"))) = {
    __SLDo512(SLDeclareBootPDE, 0x600, 0x600)
};
