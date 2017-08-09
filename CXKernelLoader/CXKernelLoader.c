extern void CXKernelLoaderMain(void);

void CXKernelLoaderMain(void)
{
    for ( ; ; ) ;
}

#if 0

In this section is pieces of code which have been removed from CXSystemLoader.
They must be reimplemented properly into the kernel loader.

From SLMemoryAllocator.c:

// On allocator initialization:
OSPrivate void SLMemoryAllocatorOnBootServicesTerminate(SLMemoryMap *finalMemoryMap, OSAddress context);

SLBootServicesRegisterTerminationFunction(SLMemoryAllocatorOnBootServicesTerminate, kOSNullPointer);

void SLMemoryAllocatorOnBootServicesTerminate(OSUnused SLMemoryMap *finalMemoryMap, OSUnused OSAddress context)
{
    gSLCurrentHeap.shouldFree = false;
}

From EFI/SLSystemTable.c:

OSPrivate CPRootDescriptor *SLSystemTableGetACPIRoot(SLSystemTable *table);

CPRootDescriptor *SLSystemTableGetACPIRoot(SLSystemTable *table)
{
    CPRootDescriptor *root = kOSNullPointer;
 
    for (OSIndex i = 0; i < table->numberOfConfigTables; i++)
    {
        SLConfigTable *configTable = &table->configTables[i];
        OSUIDIntelData acpiTableID = kSLACPITableID;
 
        if (!XKMemoryCompare(&configTable->id, &acpiTableID, sizeof(OSUIDIntelData)))
            root = configTable->pointer;
    }
 
    if (root && CPRootDescriptorValidate(root)) {
        return root;
    } else {
        return kOSNullPointer;
    }
}

From EFI/SLGraphics.c:

OSPrivate XKGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics);
OSPrivate XKGraphicsContext *SLGraphicsOutputGetContextWithMaxSize(SLGraphicsOutput *graphics, UInt32 maxHeight, UInt32 maxWidth);

XKGraphicsContext *SLGraphicsOutputGetContext(SLGraphicsOutput *graphics)
{
    return SLGraphicsOutputGetContextWithMaxSize(graphics, ~((UInt32)0), ~((UInt32)0));
}

XKGraphicsContext *SLGraphicsOutputGetContextWithMaxSize(SLGraphicsOutput *graphics, UInt32 maxHeight, UInt32 maxWidth)
{
    SLBootServicesCheck(kOSNullPointer);
    
    UInt32 modes = SLGraphicsOutputGetCurrentMode(graphics)->numberOfModes;
    SLGraphicsModeInfo *maxMode = kOSNullPointer;
    UInt32 maxModeNumber = 0;
    UInt32 maxModeHeight = 0;
    UInt32 maxModeWidth = 0;
    
    for (UInt32 i = 0; i < modes; i++)
    {
        SLGraphicsModeInfo *mode = SLGraphicsOutputGetMode(graphics, i);
        
        if (mode->format != kSLGraphicsPixelFormatRGBX8 && mode->format != kSLGraphicsPixelFormatBGRX8)
            continue;
        
        if (mode->width > maxWidth)
            continue;
        
        if (mode->width > maxModeWidth)
            maxModeWidth = mode->width;
    }
    
    for (UInt32 i = 0; i < modes; i++)
    {
        SLGraphicsModeInfo *mode = SLGraphicsOutputGetMode(graphics, i);
        
        if (mode->format != kSLGraphicsPixelFormatRGBX8 && mode->format != kSLGraphicsPixelFormatBGRX8)
            continue;
        
        if (mode->width == maxModeWidth)
        {
            if (mode->height > maxHeight)
                continue;
            
            if (mode->height > maxModeHeight)
            {
                maxModeHeight = mode->height;
                maxModeNumber = i;
                maxMode = mode;
            }
        }
    }
    
    if (!maxMode) return kOSNullPointer;
    SLStatus status = graphics->setMode(graphics, maxModeNumber);
    if (SLStatusIsError(status)) return kOSNullPointer;
    SLGraphicsMode *mode = SLGraphicsOutputGetCurrentMode(graphics);
    
    XKGraphicsContext *context = SLAllocate(sizeof(XKGraphicsContext)).address;
    context->height = mode->info->height;
    context->width = mode->info->width;
    context->framebuffer = mode->framebuffer;
    context->framebufferSize = mode->framebufferSize;
    context->pixelCount = mode->framebufferSize / sizeof(UInt32);
    context->isBGRX = (mode->info->format == kSLGraphicsPixelFormatBGRX8);
    
    return context;
}

From EFI/SLBootServices.c:

OSPrivate void SLBootServicesRegisterTerminationFunction(void (*function)(SLMemoryMap *finalMap, OSAddress context), OSAddress context);

typedef struct __SLBootServicesTerminateHandler {
    void (*function)(SLMemoryMap *finalMap, OSAddress context);
    OSAddress context;
    struct __SLBootServicesTerminateHandler *next;
} SLBootServicesTerminateHandler;

SLBootServicesTerminateHandler *gSLBootServicesFirstHandler = kOSNullPointer;

void SLBootServicesRegisterTerminationFunction(void (*function)(SLMemoryMap *finalMap, OSAddress context), OSAddress context)
{
    SLBootServicesCheck((void)(0));
    
    SLBootServicesTerminateHandler *newHandler = SLAllocate(sizeof(SLBootServicesTerminateHandler)).address;
    SLBootServicesTerminateHandler *handler = gSLBootServicesFirstHandler;
    
    if (OSExpect(handler)) {
        while (handler->next)
            handler = handler->next;
        
        handler->next = newHandler;
        handler = newHandler;
    } else {
        gSLBootServicesFirstHandler = handler = newHandler;
    }
    
    handler->function = function;
    handler->context = context;
    handler->next = kOSNullPointer;
}

SLBootServicesTerminateHandler *handler = gSLBootServicesFirstHandler;
XKLog(kXKLogLevelInfo, "Calling Boot Services Terminate Handlers...\n");

while (handler)
{
    handler->function(finalMemoryMap, handler->context);
    
    SLBootServicesTerminateHandler *oldHandler = handler;
    handler = handler->next;
    SLFree(oldHandler);
}

XKLog(kXKLogLevelInfo, "All Handlers Called; Terminating Boot Services...");

From EFI/SLFile.c:

OSUTF16Char *SLPathToEFIPath(OSUTF8Char *path)
{
    OSSize copySize = XKUTF8Length(path) + 1;
    OSUTF8Char *copy = SLAllocate(copySize).address;
    XKMemoryCopy(path, copy, copySize);
    
    for (OSIndex i = 0; i < (copySize - 1); i++)
    {
        if (copy[i] == '/')
            copy[i] = '\\';
    }
    
    OSUTF16Char *efiPath = XKUTF8ToUTF16(copy);
    SLFree(copy);
    
    return efiPath;
}

#endif
