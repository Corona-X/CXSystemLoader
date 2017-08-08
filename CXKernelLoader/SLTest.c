#include <SystemLoader/SystemLoader.h>
#include <SystemLoader/SLDebug.h>
#include <Kernel/XKShared.h>

#if 0

#if kCXBuildDev

bool SLTestFormattedPrint(void)
{
    UInt16 data[8] = {0x1234, 0x5678, 0x09AB, 0xCDEF};
    bool testFailed = false;

    #define SLTestFormattedPrintGeneralCase(f, v, s, t)                     \
        do {                                                                \
            OSUTF8Char *testString = XKPrintToString(f, v);                 \
            OSUTF8Char *testValue  = (OSUTF8Char *)t;                       \
            bool success = !XKStringCompare8(testString, testValue);        \
            const char *status = (success ? "Success" : "Failure");         \
                                                                            \
            XKLog(kXKLogLevelVerbose, "%s: %s with " s "\n", status, f);    \
            SLFree(testString);                                             \
                                                                            \
            if (!testFailed && !success)                                    \
                testFailed = true;                                          \
        } while (0)

    #define SLTestFormattedPrintCase(v, f) SLTestFormattedPrintGeneralCase(f, v, #v, #v)

    SLTestFormattedPrintCase(0x00324BCA,         "0x%08lX");
    SLTestFormattedPrintCase(07654321,           "%08o");
    SLTestFormattedPrintCase(0x0abc,             "0x%04hx");
    SLTestFormattedPrintCase(0x0F,               "0x%02hhX");
    SLTestFormattedPrintCase(0,                  "%hhu");
    SLTestFormattedPrintCase(-12,                "%hd");
    SLTestFormattedPrintCase(6547898,            "%d");
    SLTestFormattedPrintCase(0x0BCDEF9876543210, "0x%016llX");

    SLTestFormattedPrintGeneralCase("%06b",  0b011001,         "011001", "011001");
    SLTestFormattedPrintGeneralCase("%0d",   0,                "''",     "");
    SLTestFormattedPrintGeneralCase("%hhc",  'c',              "c",      "c");
    SLTestFormattedPrintGeneralCase("%zu",   (UInt64)57,       "57",     "57");
    SLTestFormattedPrintGeneralCase("%p",    (OSAddress)0x20C, "0x20C",  "0x20C");
    SLTestFormattedPrintGeneralCase("%% %%", 0,                "%% %%",  "% %");

    SLTestFormattedPrintGeneralCase("%h04H", data, "1234 5678 09AB CDEF", "1234 5678 09AB CDEF");
    SLTestFormattedPrintGeneralCase("%S", u"UTF-16 Test", "UTF-16 Test", "UTF-16 Test");
    SLTestFormattedPrintGeneralCase("%s",  "UTF-8 Test",  "UTF-8 Test",  "UTF-8 Test");

    return !testFailed;
    #undef SLTestFormattedPrintCase
}

bool SLTestAllocate(void)
{
    XKLog(kXKLogLevelVerbose, "Note: There have been %zu allocations thus far.\n", SLMemoryAllocatorGetAllocCount());
    XKLog(kXKLogLevelVerbose, "Note: There have been %zu frees thus far.\n", SLMemoryAllocatorGetFreeCount());
    SLMemoryAllocatorDumpHeapInfo();
    SLMemoryAllocatorDumpMainPool();

    OSSize usedSize = SLMemoryAllocatorGetMainPool()->usedSize;
    OSSize minimumSize = 15;
    OSBuffer buffer = SLAllocate(minimumSize);
    OSUTF8Char *bufferString = XKBufferToString(&buffer);

    XKLog(kXKLogLevelVerbose, "Allocated %s with minimum size = %u\n", bufferString, minimumSize);
    SLFree(bufferString);

    SLMemoryAllocatorDumpMainPool();

    SLFreeBuffer(buffer);
    XKLog(kXKLogLevelVerbose, "Freed last allocation.\n");
    SLMemoryAllocatorDumpMainPool();

    OSBuffer alloc32 = SLAllocate(32);
    OSUTF8Char *alloc32string = XKBufferToString(&alloc32);
    OSBuffer alloc123 = SLAllocate(123);
    OSUTF8Char *alloc123string = XKBufferToString(&alloc123);

    XKLog(kXKLogLevelVerbose, "Allocated %s with minimum size = 32\n", alloc32string);
    XKLog(kXKLogLevelVerbose, "Allocated %s with minimum size = 123\n", alloc123string);
    XKLog(kXKLogLevelVerbose, "Also allocated 2 strings to describe those allocations at %p and %p\n", alloc32string, alloc123string);
    SLMemoryAllocatorDumpMainPool();

    SLFree(alloc123string);
    SLFree(alloc32string);
    SLFreeBuffer(alloc32);
    SLFreeBuffer(alloc123);

    XKLog(kXKLogLevelVerbose, "Freed all previous test allocations.\n");

    SLMemoryAllocatorDumpMainPool();
    SLMemoryAllocatorDumpHeapInfo();

    bool sameSize = usedSize == SLMemoryAllocatorGetMainPool()->usedSize;

    return sameSize;
}

bool SLTestMemoryOps(void)
{
    UInt8 data[2] = {0xCA, 0xDB};
    SInt16 difference = data[0] - data[1];
    OSSize bufferSize = 256;
    bool testFailed = false;

    OSAddress buffers[2] = {
        SLAllocate(bufferSize).address,
        SLAllocate(bufferSize).address
    };

    XKMemorySetValue(buffers[0], bufferSize, data[0]);
    XKMemorySetValue(buffers[1], bufferSize, data[1]);

    SInt16 result = XKMemoryCompare(buffers[0], buffers[1], bufferSize);
    XKLog(kXKLogLevelVerbose, "Memory Compare: %hd (%hd)\n", result, difference);

    if (result != difference)
        testFailed = true;

    XKLog(kXKLogLevelVerbose, "Dumping %zu bytes from each buffer...\n", bufferSize);
    XKMemoryCopy(buffers[0], buffers[1], bufferSize);

    for (OSCount i = 0; i < bufferSize; i += (4 * sizeof(UInt64)))
    {
        OSAddress pointer0 = &((UInt8 *)(buffers[0]))[i];
        OSAddress pointer1 = &((UInt8 *)(buffers[1]))[i];

        XKLog(kXKLogLevelVerbose, "%04zH\n", pointer0);
        XKLog(kXKLogLevelVerbose, "%04zH\n", pointer1);
    }

    result = XKMemoryCompare(buffers[0], buffers[1], bufferSize);
    XKLog(kXKLogLevelVerbose, "Memory Compare: %hd (0)\n", result);

    if (result)
        testFailed = true;

    SLFree(buffers[0]);
    SLFree(buffers[1]);

    return !testFailed;
}

bool SLTestStringLengths(void)
{
    OSSize results[5] = {0, 1, 3, 11, 56};
    bool testFailed = false;
    results[0] = 0;

    #define SLPrintStringTest(l, p)                                                             \
        do {                                                                                    \
            UInt ## l *strings[5] = {                                                           \
                (UInt ## l *)p ## "",                                                           \
                (UInt ## l *)p ## "a",                                                          \
                (UInt ## l *)p ## "abc",                                                        \
                (UInt ## l *)p ## "Apple Juice",                                                \
                (UInt ## l *)p ## "Apple Juice Tastes Really Good, oh yes oh yes it does!!!"    \
            };                                                                                  \
                                                                                                \
            XKPrintString("UTF-" #l ":\n");                                                     \
                                                                                                \
            for (OSIndex i = 0; i < 5; i++)                                                     \
            {                                                                                   \
                OSSize result = XKUTF ## l ## Length(strings[i]);                               \
                                                                                                \
                XKLog(kXKLogLevelVerbose, "%zu (%zu)", result, results[i]);                     \
                if (i != 4) XKPrintString(", ");                                                \
                else XKPrintString("\n");                                                       \
                                                                                                \
                if (!testFailed && (result != results[i]))                                      \
                    testFailed = true;                                                          \
            }                                                                                   \
        } while (0)

    //XKPrintString("%d\n", results[0]);

    SLPrintStringTest(32, U);
    SLPrintStringTest(16, u);
    SLPrintStringTest(8,   );

    return !testFailed;
    #undef SLPrintStringTest
}

void SLRunTest(bool (*test)(void))
{
    gXKLogLevelCurrent = kXKLogLevelVerbose;
    XKPrintString("\n");

    bool result = test();

    if (result) XKLog(kXKLogLevelInfo,    "All Test Cases Passed.\n");
    else        XKLog(kXKLogLevelWarning, "One or More Test Cases Failed!\n");
}

void SLRunTests(void)
{
    XKLog(kXKLogLevelInfo, "Running Formatted Print Tests...  ");
    SLRunTest(SLTestFormattedPrint);

    XKLog(kXKLogLevelInfo, "Running Memory Allocator Tests... ");
    SLRunTest(SLTestAllocate);

    XKLog(kXKLogLevelInfo, "Running Memory Operation Tests... ");
    SLRunTest(SLTestMemoryOps);

    XKLog(kXKLogLevelInfo, "Running String Length Tests...    ");
    SLRunTest(SLTestStringLengths);
}

#endif /* kCXBuildDev */

#endif
