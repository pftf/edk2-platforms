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

#include "GenetReg.h"
#include "GenericPhy.h"

#define GENET_VERSION             0x0a

#define GENET_MAX_PACKET_SIZE     2048

typedef struct {
  MAC_ADDR_DEVICE_PATH            MacAddrDP;
  EFI_DEVICE_PATH_PROTOCOL        End;
} GENET_DEVICE_PATH;

typedef struct {
  EFI_PHYSICAL_ADDRESS            Pa;
  VOID *                          Mapping;
} GENET_MAP_INFO;

typedef enum {
  GENET_PHY_MODE_MII,
  GENET_PHY_MODE_RGMII,
  GENET_PHY_MODE_RGMII_RXID,
} GENET_PHY_MODE;

typedef struct {
  UINT32                          Signature;
  EFI_HANDLE                      ControllerHandle;

  EFI_LOCK                        Lock;

  EFI_SIMPLE_NETWORK_PROTOCOL     Snp;
  EFI_SIMPLE_NETWORK_MODE         SnpMode;

  VOID                            *Dev;

  GENERIC_PHY_PRIVATE_DATA        Phy;

  UINT8 *                         TxBuffer[GENET_DMA_DESC_COUNT];
  UINT8                           TxQueued;
  UINT16                          TxNext;
  UINT16                          TxConsIndex;
  UINT16                          TxProdIndex;

  UINT8 *                         RxBuffer[GENET_DMA_DESC_COUNT];
  GENET_MAP_INFO                  RxBufferMap[GENET_DMA_DESC_COUNT];
  UINT16                          RxConsIndex;
  UINT16                          RxProdIndex;

  GENET_PHY_MODE                  PhyMode;

  UINTN                           RegBase;
} GENET_PRIVATE_DATA;

extern EFI_COMPONENT_NAME_PROTOCOL            gGenetComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL           gGenetComponentName2;

#define GENET_DRIVER_SIGNATURE                SIGNATURE_32('G', 'N', 'E', 'T')
#define GENET_PRIVATE_DATA_FROM_SNP_THIS(a)   CR(a, GENET_PRIVATE_DATA, Snp, GENET_DRIVER_SIGNATURE)

UINT32
EFIAPI
GenetMmioRead (
  GENET_PRIVATE_DATA      *Genet,
  UINT32                  Offset
  );

VOID
EFIAPI
GenetMmioWrite (
  GENET_PRIVATE_DATA      *Genet,
  UINT32                  Offset,
  UINT32                  Data
  );

EFI_STATUS
EFIAPI
GenetPhyRead (
  IN VOID                 *Priv,
  IN UINT8                PhyAddr,
  IN UINT8                Reg,
  OUT UINT16              *Data
  );

EFI_STATUS
EFIAPI
GenetPhyWrite (
  IN VOID                 *Priv,
  IN UINT8                PhyAddr,
  IN UINT8                Reg,
  IN UINT16               Data
  );

VOID
EFIAPI
GenetPhyConfigure (
  IN VOID                 *Priv,
  IN GENERIC_PHY_SPEED    Speed,
  IN GENERIC_PHY_DUPLEX   Duplex
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
  IN GENET_PRIVATE_DATA * Genet
  );

VOID
EFIAPI
GenetDisableTxRx (
  IN GENET_PRIVATE_DATA * Genet
  );

VOID
EFIAPI
GenetSetPromisc (
  IN GENET_PRIVATE_DATA * Genet,
  IN BOOLEAN              Enable
  );

EFI_STATUS
EFIAPI
GenetDmaInitRings (
  IN GENET_PRIVATE_DATA * Genet
  );

EFI_STATUS
EFIAPI
GenetDmaAlloc (
  IN GENET_PRIVATE_DATA * Genet
  );

VOID
EFIAPI
GenetDmaFree (
  IN GENET_PRIVATE_DATA * Genet
  );

VOID
EFIAPI
GenetDmaTriggerTx (
  IN GENET_PRIVATE_DATA * Genet,
  IN UINT8                DescIndex,
  IN EFI_PHYSICAL_ADDRESS PhysAddr,
  IN UINTN                NumberOfBytes
  );

EFI_STATUS
EFIAPI
GenetDmaMapRxDescriptor (
  IN GENET_PRIVATE_DATA * Genet,
  IN UINT8                DescIndex
  );

VOID
EFIAPI
GenetDmaUnmapRxDescriptor (
  IN GENET_PRIVATE_DATA * Genet,
  IN UINT8                DescIndex
  );

VOID
EFIAPI
GenetTxIntr (
  IN GENET_PRIVATE_DATA * Genet,
  OUT VOID **             TxBuf
  );

EFI_STATUS
EFIAPI
GenetRxIntr (
  IN GENET_PRIVATE_DATA * Genet,
  OUT UINT8 *             DescIndex,
  OUT UINTN *             FrameLength
  );

#endif /* BCM_GENET_H__ */
