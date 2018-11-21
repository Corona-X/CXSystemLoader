/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SystemLoader.h - Include for all SystemLoader headers           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 26.9.2016  - 9:00 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER__
#define __SYSTEMLOADER__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLMach-O.h>
#include <SystemLoader/SLMemoryAllocator.h>

#include <SystemLoader/EFI/SLBlockIO.h>
#include <SystemLoader/EFI/SLBootConsole.h>
#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLFile.h>
#include <SystemLoader/EFI/SLGraphics.h>
#include <SystemLoader/EFI/SLLoadedImage.h>
#include <SystemLoader/EFI/SLRuntimeServices.h>
#include <SystemLoader/EFI/SLSystemTable.h>

#include <SystemLoader/Kernel/SLDebug.h>
#include <SystemLoader/Kernel/SLSMBIOS.h>
#include <SystemLoader/Kernel/SLKernelLoader.h>

#include <SystemLoader/Loader/SLConfig.h>
#include <SystemLoader/Loader/SLSystemLoader.h>
#include <SystemLoader/Loader/SLLoaderIO.h>
#include <SystemLoader/Loader/SLLoader.h>

#endif /* !defined(__SYSTEM__) */
