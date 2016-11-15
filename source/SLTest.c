#include <SystemLoader/SystemLoader.h>
#include <SystemLoader/SLLibrary.h>

#if kCXBuildDev

void SLTestFormattedPrint(void)
{
    UInt16 data[8] = {0x1234, 0x5678, 0x09AB, 0xCDEF};

    SLPrintString("0x00324BCA\n0x%08lX\n",           0x00324BCA);
    SLPrintString("07654321\n%08o\n",                07654321);
    SLPrintString("0x0abc\n0x%04hx\n",               0x0abc);
    SLPrintString("0x0F\n0x%02hhX\n",                0x0F);
    SLPrintString("0\n%hhu\n",                       0);
    SLPrintString("-12\n%hd\n",                      -12);
    SLPrintString("011001\n%06b\n",                  0b011001);
    SLPrintString("c\n%hhc\n",                       'c');
    SLPrintString(" \n%0d\n",                        0);
    SLPrintString("6547898\n%d\n",                   6547898);
    SLPrintString("57\n%zu\n",                       (UInt64)57);
    SLPrintString("0x0BCDEF9876543210\n0x%016llX\n", 0x0BCDEF9876543210);
    SLPrintString("UTF-16 Test\n%S\n",               u"UTF-16 Test");
    SLPrintString("UTF-8  Test\n%s\n",                "UTF-8  Test");
    SLPrintString("0x20C\n%p\n",                     (OSAddress)0x20C);
    SLPrintString("1234 5678 09AB CDEF\n%h04H\n",    data);
    SLPrintString("%%\n%%\n");
    SLPrintString("\n");
}

void SLDumpHeapInfo(SLHeap *heap)
{
    SLPrintString("Heap Info:\n");
    SLPrintString("Base Address: %p\n", heap->baseAddress);
    SLPrintString("Current Size: 0x%zX [%zu]\n", heap->currentSize, heap->currentSize);
    SLPrintString("Maximum Size: 0x%zX [%zu]\n", heap->maxSize, heap->maxSize);
    SLPrintString("Will Free:    %s\n", (heap->shouldFree ? "Yes" : "No"));
    SLPrintString("\n");
}

void SLDumpPoolInfo(SLMemoryPool *pool)
{
    SLPrintString("Pool Base: %p\n", pool->address);
    SLPrintString("Pool Size: %zX [%zu]\n", pool->size, pool->size);
    SLPrintString("Used Size: %zX [%zu]\n", pool->usedSize, pool->usedSize);
    SLPrintString("Node List:\n");

    SLMemoryNode *node = pool->head;
    OSCount spaces = 0;

    while (node)
    {
        for (OSCount i = 0; i < spaces; i++)
            SLPrintString(" ");

        SLPrintString("--> %p (%zX [%zu] bytes) --> %p\n", node, node->size, node->size, node->next);
        node = node->next;
        spaces++;
    }

    SLPrintString("\n");
}

void SLTestAllocate(void)
{
    OSAddress alloc0base, alloc1base, alloc2base;
    OSAddress alloc0size, alloc1size, alloc2size;

    SLDumpHeapInfo(SLMemoryAllocatorGetRealHeap());
    SLDumpPoolInfo(SLMemoryAllocatorGetMainPool());

    OSBuffer a = SLAllocate(15);
    alloc0base = a.address;
    alloc0size = a.size;
    SLPrintString("Got %zu bytes at %p\n", alloc0size, alloc0base);
    SLDumpPoolInfo(SLMemoryAllocatorGetMainPool());
    SLFree(a.address);

    SLDumpPoolInfo(SLMemoryAllocatorGetMainPool());

    OSBuffer b = SLAllocate(32);
    OSBuffer c = SLAllocate(15);
    alloc1base = b.address;
    alloc1size = b.size;
    alloc2base = c.address;
    alloc2size = c.size;
    SLPrintString("Got {%p, %zu} and {%p, %zu}\n", alloc1base, alloc1size, alloc2base, alloc2size);
    SLDumpPoolInfo(SLMemoryAllocatorGetMainPool());
    SLFree(b.address);
    SLDumpPoolInfo(SLMemoryAllocatorGetMainPool());
    SLFree(c.address);

    SLDumpPoolInfo(SLMemoryAllocatorGetMainPool());
}

bool SLICompare(const char *restrict s1, const char *restrict s2)
{
    for ( ; (*s1) == (*s2); s1++, s2++);
    return !((*s1) ^ (*s2));
}

void SLTestMemoryOps(void)
{
    OSSize s = 256;

    OSBuffer a = SLAllocate(s);
    OSBuffer b = SLAllocate(s);

    CXKMemorySetValue(a.address, s, 0xCA);
    CXKMemorySetValue(b.address, s, 0xDB);
    SLPrintString("Memory Compare: %hd\n", CXKMemoryCompare(a.address, b.address, s));

    CXKMemoryCopy(a.address, b.address, s);
    OSAddress aa = a.address;
    OSAddress ba = b.address;

    for (OSCount i = 0; i < s; i += (4 * sizeof(UInt64)))
    {
        UInt64 *ap = (UInt64 *)aa;
        UInt64 *bp = (UInt64 *)ba;

        SLPrintString("%04llH\n%04llH\n", ap, bp);

        aa += (4 * sizeof(UInt64));
        ba += (4 * sizeof(UInt64));
    }

    SLPrintString("Memory Compare: %hd\n\n", CXKMemoryCompare(a.address, b.address, s));
    SLFree(b.address);
    SLFree(a.address);
}

#define SLPrintStringTest(l) SLPrintString("%zu, %zu, %zu, %zu, %zu\n", CXKStringSize ## l(utf ## l ## string0), CXKStringSize ## l(utf ## l ## string1), CXKStringSize ## l(utf ## l ## string2), CXKStringSize ## l(utf ## l ## string3), CXKStringSize ## l(utf ## l ## string4))

void SLTestStringLengths(void)
{
    UInt32 *utf32string0 = U"";
    UInt32 *utf32string1 = U"a";
    UInt32 *utf32string2 = U"abc";
    UInt32 *utf32string3 = U"Apple Juice";
    UInt32 *utf32string4 = U"Apple Juice Tastes Really Good, oh yes oh yes it does!!!";

    SLPrintString("0, 1, 3, 11, 56\n");
    SLPrintString("UTF-32:\n");
    SLPrintStringTest(32);

    UInt16 *utf16string0 = u"";
    UInt16 *utf16string1 = u"a";
    UInt16 *utf16string2 = u"abc";
    UInt16 *utf16string3 = u"Apple Juice";
    UInt16 *utf16string4 = u"Apple Juice Tastes Really Good, oh yes oh yes it does!!!";

    SLPrintString("UTF-16:\n");
    SLPrintStringTest(16);

    UInt8 *utf8string0 = (UInt8 *)"";
    UInt8 *utf8string1 = (UInt8 *)"a";
    UInt8 *utf8string2 = (UInt8 *)"abc";
    UInt8 *utf8string3 = (UInt8 *)"Apple Juice";
    UInt8 *utf8string4 = (UInt8 *)"Apple Juice Tastes Really Good, oh yes oh yes it does!!!";

    SLPrintString("UTF-8:\n");
    SLPrintStringTest(8);
}

void SLRunTests(void)
{
    //SLFree(SLAllocate(8192));
    CXKMemoryMap *map = SLBootServicesTerminate();
    SLPrintString("!!!Terminated Boot Services [final memory map @ %p]!!!\n", map);

    SLPrintString("Running Formatted Print Test...\n\n");
    SLTestFormattedPrint();

    SLPrintString("Running Memory Op Tests...\n\n");
    SLTestMemoryOps();

    SLPrintString("Running Allocate Test...\n\n");
    SLTestAllocate();

    SLPrintString("Running String Length Tests...\n");
    SLTestStringLengths();
}

#endif /* kCXBuildDev */
