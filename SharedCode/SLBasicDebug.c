#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLBasicDebug.h>
#include <SystemLoader/SLBasicIO.h>

void SLMemoryAllocatorDumpHeapInfo(void)
{
    SLPrintString("Heap Info:\n");
    SLPrintString("Base Address: %p\n", gSLCurrentHeap.baseAddress);
    SLPrintString("Size: %u/%u ",       gSLCurrentHeap.currentSize, gSLCurrentHeap.maxSize);
    SLPrintString("(0x%zX/0x%zX)\n",    gSLCurrentHeap.currentSize, gSLCurrentHeap.maxSize);
    SLPrintString("Will Free:    %s\n", (gSLCurrentHeap.shouldFree ? "yes" : "no"));
}

void SLMemoryAllocatorDumpMainPool(void)
{
    SLPrintString("Pool Info:\n");
    SLPrintString("Base Address: %p\n", gSLMainPoolInfo.address);
    SLPrintString("Size: %u/%u ",       gSLMainPoolInfo.usedSize, gSLMainPoolInfo.size);
    SLPrintString("(0x%zX/0x%zX)\n",    gSLMainPoolInfo.usedSize, gSLMainPoolInfo.size);
    SLPrintString("Node List:\n");

    SLMemoryNode *node = gSLMainPoolInfo.head;
    OSCount spaces = 0;

    while (node)
    {
        SLPrintString("");

        for (OSCount i = 0; i < spaces; i++)
            SLPrintString(" ");

        SLPrintString("--> %p (0x%zX bytes) --> %p\n", node, node->size, node->next);
        node = node->next;
        spaces += 2;
    }
}
