/** @file

  Copyright (c) 2019, Jeremy Linton All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  This driver acts like a stub to set the BCM 
  Genet MAC address, until the actual network driver
  is in place. This should allow us to retrieve the
  mac address directly from the hardware in supported
  OS's rather than passing it via DSDT (which isn't 
  ideal for a number of reasons, foremost the hardware
  should be self describing).

**/

#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <PiDxe.h>
#include <Protocol/RpiFirmware.h>

#define GENET_BASE                 0xfd580000  // len = 0x10000
#define GENET_SYS_RBUF_FLUSH_CTRL  0x0008
#define GENET_UMAC_MAC0            0x080C
#define GENET_UMAC_MAC1            0x0810

STATIC
VOID
RMWRegister (
  UINT32                Offset,
  UINT32                Mask,
  UINT32                In
  )
{
  EFI_PHYSICAL_ADDRESS  Addr = GENET_BASE;
  UINT32                Data = 0;
  UINT32                Shift;

  Addr += Offset;
  Shift = 1;
  if (In) {
    while (!(Mask & Shift))
      Shift <<= 1;
    Data = (MmioRead32 (Addr) & ~Mask) | ((In * Shift) & Mask);
  } else {
    Data = MmioRead32 (Addr) & ~Mask;
  }

  MmioWrite32 (Addr, Data);

  ArmDataMemoryBarrier ();
}

STATIC
VOID
WdRegister (
  UINT32                Offset,
  UINT32                In
  )
{
  EFI_PHYSICAL_ADDRESS  Base = GENET_BASE;

  MmioWrite32 (Base + Offset, In);

  ArmDataMemoryBarrier ();
}



STATIC VOID
GenetMacInit (UINT8 *addr)
{

  // bring the umac out of reset
  RMWRegister (GENET_SYS_RBUF_FLUSH_CTRL, 0x2, 1);
  gBS->Stall (10);
  RMWRegister (GENET_SYS_RBUF_FLUSH_CTRL, 0x2, 0);

  // Update the MAC

  WdRegister(GENET_UMAC_MAC0, (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | addr[3]);
  WdRegister(GENET_UMAC_MAC1, (addr[4] << 8) | addr[5]);
}

/**
  The entry point of Genet UEFI Driver.

  @param  ImageHandle                The image handle of the UEFI Driver.
  @param  SystemTable                A pointer to the EFI System Table.

  @retval  EFI_SUCCESS               The Driver or UEFI Driver exited normally.
  @retval  EFI_INCOMPATIBLE_VERSION  _gUefiDriverRevision is greater than
                                     SystemTable->Hdr.Revision.

**/
EFI_STATUS
EFIAPI
GenetEntryPoint (
  IN  EFI_HANDLE          ImageHandle,
  IN  EFI_SYSTEM_TABLE    *SystemTable
  )
{
  RASPBERRY_PI_FIRMWARE_PROTOCOL   *mFwProtocol;
  EFI_STATUS Status;
  UINT8 MacAddr[6];

  DEBUG ((DEBUG_ERROR, "GENET:%a(): Init\n", __FUNCTION__));

  Status = gBS->LocateProtocol (&gRaspberryPiFirmwareProtocolGuid, NULL,
                  (VOID**)&mFwProtocol);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "%a: failed to locate RPi firmware protocol\n", __FUNCTION__));
    return Status;
  }
  
  // Get the MAC address from the firmware
  Status = mFwProtocol->GetMacAddress (MacAddr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: failed to retrieve MAC address\n", __FUNCTION__));
    return Status;
  }

  // Write it to the hardware
  GenetMacInit (MacAddr);

  return EFI_SUCCESS;
}
