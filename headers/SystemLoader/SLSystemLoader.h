/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLSystemLoader.h - Routines for loading a CX System Image (CAR) */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.8.2016   -  3:15 PM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLSYSTEMLOADER__
#define __SYSTEMLOADER_SLSYSTEMLOADER__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/EFI/SLBlockIO.h>
#include <System/Archives/OSCAR.h>

#if !kCXAssemblyCode && kCXBootloaderCode

OSPrivate UInt64 SLIsSystemPartition(SLBlockIO *blockDevice);
OSPrivate void SLPrintSystemVersionInfo(CASystemVersionInternal *version);
OSPrivate OSNoReturn void SLLoadSystemOrLeave(SLBlockIO *blockDevice);

#endif /* kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLSYSTEMLOADER__) */
