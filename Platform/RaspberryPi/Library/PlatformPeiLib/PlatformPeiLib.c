/** @file
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <PiPei.h>

#include <Library/ArmPlatformLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Guid/DualSerialPortLibHobGuid.h>

extern UINT32 gSerialLibCoreClockFreq, gSerialLibCoreSkipHob;

static DUAL_SERIAL_PORT_LIB_VARS mUartLibVars;

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  )
{
  BuildFvHob (PcdGet64 (PcdFvBaseAddress), PcdGet32 (PcdFvSize));

  mUartLibVars.CoreClockFreq = gSerialLibCoreClockFreq;
  BuildGuidDataHob (&gDualSerialPortLibHobGuid, &mUartLibVars, sizeof(mUartLibVars));

  return EFI_SUCCESS;
}
