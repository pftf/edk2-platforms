/** @file
  Copyright (c) 2020 Jared McNeill <jmcneill@invisible.ca>

  SPDX-License-Identifier: BSD-2-Clause
**/

#ifndef BCM_GENET_H__
#define BCM_GENET_H__

#include <Uefi.h>

#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/SimpleNetwork.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/NetLib.h>
#include <Library/UefiLib.h>

#define GENET_VERSION             0x0a

typedef struct {
  MAC_ADDR_DEVICE_PATH            MacAddrDP;
  EFI_DEVICE_PATH_PROTOCOL        End;
} GENET_DEVICE_PATH;

typedef struct {
  UINT32                          Signature;
  EFI_HANDLE                      ControllerHandle;

  EFI_SIMPLE_NETWORK_PROTOCOL     Snp;
  EFI_SIMPLE_NETWORK_MODE         SnpMode;

  VOID                            *Dev;

  UINT8                           PhyAddr;
  BOOLEAN                         PhyLinkUp;

  UINTN                           RegBase;
} GENET_PRIVATE_DATA;

typedef enum {
  GENET_PHY_MODE_MII,
  GENET_PHY_MODE_RGMII,
  GENET_PHY_MODE_RGMII_RXID,
} GENET_PHY_MODE;

extern EFI_COMPONENT_NAME_PROTOCOL            gGenetComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL           gGenetComponentName2;

#define GENET_DRIVER_SIGNATURE                SIGNATURE_32('G', 'N', 'E', 'T')
#define GENET_PRIVATE_DATA_FROM_SNP_THIS(a)   CR(a, GENET_PRIVATE_DATA, Snp, GENET_DRIVER_SIGNATURE)

UINT32
EFIAPI
GenetMmioRead (
  GENET_PRIVATE_DATA *  Genet,
  UINT32                Offset
  );

VOID
EFIAPI
GenetMmioWrite (
  GENET_PRIVATE_DATA *  Genet,
  UINT32                Offset,
  UINT32                Data
  );

EFI_STATUS
EFIAPI
GenetPhyInit (
  IN GENET_PRIVATE_DATA * Genet
  );

EFI_STATUS
EFIAPI
GenetPhyReset (
  IN GENET_PRIVATE_DATA * Genet
  );

EFI_STATUS
EFIAPI
GenetPhyUpdateConfig (
  IN GENET_PRIVATE_DATA * Genet
  );

VOID
EFIAPI
GenetReset (
  IN GENET_PRIVATE_DATA * Genet
  );

VOID
EFIAPI
GenetSetMacAddress (
  IN GENET_PRIVATE_DATA * Genet,
  IN EFI_MAC_ADDRESS *    MacAddr
  );

VOID
EFIAPI
GenetSetPhyMode (
  IN GENET_PRIVATE_DATA * Genet,
  IN GENET_PHY_MODE       PhyMode
  );

VOID
EFIAPI
GenetEnableTxRx (
  IN GENET_PRIVATE_DATA * Genet,
  IN BOOLEAN              Enable
  );

VOID
EFIAPI
GenetSetPromisc (
  IN GENET_PRIVATE_DATA * Genet,
  IN BOOLEAN              Enable
  );

#endif /* BCM_GENET_H__ */
