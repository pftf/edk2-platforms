/** @file

  Copyright (c) 2020 Jared McNeill <jmcneill@invisible.ca>
  Copyright (c) 2020, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef GENET_UTIL_H__
#define GENET_UTIL_H__

#include <Uefi.h>

#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/BcmGenetPlatformDevice.h>
#include <Protocol/SimpleNetwork.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/NetLib.h>
#include <Library/UefiLib.h>

#include "GenericPhy.h"

#define GENET_VERSION                           0x0a
#define GENET_MAX_PACKET_SIZE                   2048

#define GENET_SYS_REV_CTRL                      0x000
#define  SYS_REV_MAJOR                          (BIT27|BIT26|BIT25|BIT24)
#define  SYS_REV_MINOR                          (BIT19|BIT18|BIT17|BIT16)
#define GENET_SYS_PORT_CTRL                     0x004
#define  GENET_SYS_PORT_MODE_EXT_GPHY           3
#define GENET_SYS_RBUF_FLUSH_CTRL               0x008
#define  GENET_SYS_RBUF_FLUSH_RESET             BIT1
#define GENET_SYS_TBUF_FLUSH_CTRL               0x00c
#define GENET_EXT_RGMII_OOB_CTRL                0x08c
#define  GENET_EXT_RGMII_OOB_ID_MODE_DISABLE    BIT16
#define  GENET_EXT_RGMII_OOB_RGMII_MODE_EN      BIT6
#define  GENET_EXT_RGMII_OOB_OOB_DISABLE        BIT5
#define  GENET_EXT_RGMII_OOB_RGMII_LINK         BIT4
#define GENET_INTRL2_CPU_STAT                   0x200
#define GENET_INTRL2_CPU_CLEAR                  0x208
#define GENET_INTRL2_CPU_STAT_MASK              0x20c
#define GENET_INTRL2_CPU_SET_MASK               0x210
#define GENET_INTRL2_CPU_CLEAR_MASK             0x214
#define  GENET_IRQ_MDIO_ERROR                   BIT24
#define  GENET_IRQ_MDIO_DONE                    BIT23
#define  GENET_IRQ_TXDMA_DONE                   BIT16
#define  GENET_IRQ_RXDMA_DONE                   BIT13
#define GENET_RBUF_CTRL                         0x300
#define  GENET_RBUF_BAD_DIS                     BIT2
#define  GENET_RBUF_ALIGN_2B                    BIT1
#define  GENET_RBUF_64B_EN                      BIT0
#define GENET_RBUF_TBUF_SIZE_CTRL               0x3b4
#define GENET_UMAC_CMD                          0x808
#define  GENET_UMAC_CMD_LCL_LOOP_EN             BIT15
#define  GENET_UMAC_CMD_SW_RESET                BIT13
#define  GENET_UMAC_CMD_HD_EN                   BIT10
#define  GENET_UMAC_CMD_PROMISC                 BIT4
#define  GENET_UMAC_CMD_SPEED                   (BIT3|BIT2)
#define   GENET_UMAC_CMD_SPEED_10               0
#define   GENET_UMAC_CMD_SPEED_100              1
#define   GENET_UMAC_CMD_SPEED_1000             2
#define  GENET_UMAC_CMD_RXEN                    BIT1
#define  GENET_UMAC_CMD_TXEN                    BIT0
#define GENET_UMAC_MAC0                         0x80c
#define GENET_UMAC_MAC1                         0x810
#define GENET_UMAC_MAX_FRAME_LEN                0x814
#define GENET_UMAC_TX_FLUSH                     0xb34
#define GENET_UMAC_MIB_CTRL                     0xd80
#define  GENET_UMAC_MIB_RESET_TX                BIT2
#define  GENET_UMAC_MIB_RESET_RUNT              BIT1
#define  GENET_UMAC_MIB_RESET_RX                BIT0
#define GENET_MDIO_CMD                          0xe14
#define  GENET_MDIO_START_BUSY                  BIT29
#define  GENET_MDIO_READ                        BIT27
#define  GENET_MDIO_WRITE                       BIT26
#define  GENET_MDIO_PMD                         (BIT25|BIT24|BIT23|BIT22|BIT21)
#define  GENET_MDIO_REG                         (BIT20|BIT19|BIT18|BIT17|BIT16)
#define GENET_UMAC_MDF_CTRL                     0xe50
#define GENET_UMAC_MDF_ADDR0(n)                 (0xe54 + (n) * 0x8)
#define GENET_UMAC_MDF_ADDR1(n)                 (0xe58 + (n) * 0x8)

#define GENET_DMA_DESC_COUNT                    256
#define GENET_DMA_DESC_SIZE                     12
#define GENET_DMA_DEFAULT_QUEUE                 16

#define GENET_DMA_RING_SIZE                     0x40
#define GENET_DMA_RINGS_SIZE                    (GENET_DMA_RING_SIZE * (GENET_DMA_DEFAULT_QUEUE + 1))

#define GENET_RX_BASE                           0x2000
#define GENET_TX_BASE                           0x4000

#define GENET_RX_DMA_RINGBASE(qid)              (GENET_RX_BASE + 0xc00 + GENET_DMA_RING_SIZE * (qid))
#define GENET_RX_DMA_WRITE_PTR_LO(qid)          (GENET_RX_DMA_RINGBASE(qid) + 0x00)
#define GENET_RX_DMA_WRITE_PTR_HI(qid)          (GENET_RX_DMA_RINGBASE(qid) + 0x04)
#define GENET_RX_DMA_PROD_INDEX(qid)            (GENET_RX_DMA_RINGBASE(qid) + 0x08)
#define GENET_RX_DMA_CONS_INDEX(qid)            (GENET_RX_DMA_RINGBASE(qid) + 0x0c)
#define GENET_RX_DMA_RING_BUF_SIZE(qid)         (GENET_RX_DMA_RINGBASE(qid) + 0x10)
#define  GENET_RX_DMA_RING_BUF_SIZE_DESC_COUNT  0xffff0000
#define  GENET_RX_DMA_RING_BUF_SIZE_BUF_LENGTH  0x0000ffff
#define GENET_RX_DMA_START_ADDR_LO(qid)         (GENET_RX_DMA_RINGBASE(qid) + 0x14)
#define GENET_RX_DMA_START_ADDR_HI(qid)         (GENET_RX_DMA_RINGBASE(qid) + 0x18)
#define GENET_RX_DMA_END_ADDR_LO(qid)           (GENET_RX_DMA_RINGBASE(qid) + 0x1c)
#define GENET_RX_DMA_END_ADDR_HI(qid)           (GENET_RX_DMA_RINGBASE(qid) + 0x20)
#define GENET_RX_DMA_XON_XOFF_THRES(qid)        (GENET_RX_DMA_RINGBASE(qid) + 0x28)
#define  GENET_RX_DMA_XON_XOFF_THRES_LO         0xffff0000
#define  GENET_RX_DMA_XON_XOFF_THRES_HI         0x0000ffff
#define GENET_RX_DMA_READ_PTR_LO(qid)           (GENET_RX_DMA_RINGBASE(qid) + 0x2c)
#define GENET_RX_DMA_READ_PTR_HI(qid)           (GENET_RX_DMA_RINGBASE(qid) + 0x30)

#define GENET_TX_DMA_RINGBASE(qid)              (GENET_TX_BASE + 0xc00 + GENET_DMA_RING_SIZE * (qid))
#define GENET_TX_DMA_READ_PTR_LO(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x00)
#define GENET_TX_DMA_READ_PTR_HI(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x04)
#define GENET_TX_DMA_CONS_INDEX(qid)            (GENET_TX_DMA_RINGBASE(qid) + 0x08)
#define GENET_TX_DMA_PROD_INDEX(qid)            (GENET_TX_DMA_RINGBASE(qid) + 0x0c)
#define GENET_TX_DMA_RING_BUF_SIZE(qid)         (GENET_TX_DMA_RINGBASE(qid) + 0x10)
#define  GENET_TX_DMA_RING_BUF_SIZE_DESC_COUNT  0xffff0000
#define  GENET_TX_DMA_RING_BUF_SIZE_BUF_LENGTH  0x0000ffff
#define GENET_TX_DMA_START_ADDR_LO(qid)         (GENET_TX_DMA_RINGBASE(qid) + 0x14)
#define GENET_TX_DMA_START_ADDR_HI(qid)         (GENET_TX_DMA_RINGBASE(qid) + 0x18)
#define GENET_TX_DMA_END_ADDR_LO(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x1c)
#define GENET_TX_DMA_END_ADDR_HI(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x20)
#define GENET_TX_DMA_MBUF_DONE_THRES(qid)       (GENET_TX_DMA_RINGBASE(qid) + 0x24)
#define GENET_TX_DMA_FLOW_PERIOD(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x28)
#define GENET_TX_DMA_WRITE_PTR_LO(qid)          (GENET_TX_DMA_RINGBASE(qid) + 0x2c)
#define GENET_TX_DMA_WRITE_PTR_HI(qid)          (GENET_TX_DMA_RINGBASE(qid) + 0x30)

#define GENET_RX_DESC_STATUS(idx)               (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x00)
#define  GENET_RX_DESC_STATUS_BUFLEN            (BIT27|BIT26|BIT25|BIT24|BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
#define  GENET_RX_DESC_STATUS_OWN               BIT15
#define  GENET_RX_DESC_STATUS_EOP               BIT14
#define  GENET_RX_DESC_STATUS_SOP               BIT13
#define  GENET_RX_DESC_STATUS_RX_ERROR          BIT2
#define GENET_RX_DESC_ADDRESS_LO(idx)           (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x04)
#define GENET_RX_DESC_ADDRESS_HI(idx)           (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x08)

#define GENET_TX_DESC_STATUS(idx)               (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x00)
#define  GENET_TX_DESC_STATUS_BUFLEN            (BIT27|BIT26|BIT25|BIT24|BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
#define  GENET_TX_DESC_STATUS_OWN               BIT15
#define  GENET_TX_DESC_STATUS_EOP               BIT14
#define  GENET_TX_DESC_STATUS_SOP               BIT13
#define  GENET_TX_DESC_STATUS_QTAG              (BIT12|BIT11|BIT10|BIT9|BIT8|BIT7)
#define  GENET_TX_DESC_STATUS_CRC               BIT6
#define GENET_TX_DESC_ADDRESS_LO(idx)           (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x04)
#define GENET_TX_DESC_ADDRESS_HI(idx)           (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x08)

#define GENET_RX_DMA_RING_CFG                   (GENET_RX_BASE + 0x1040 + 0x00)
#define GENET_RX_DMA_CTRL                       (GENET_RX_BASE + 0x1040 + 0x04)
#define  GENET_RX_DMA_CTRL_RBUF_EN(qid)         (BIT1 << (qid))
#define  GENET_RX_DMA_CTRL_EN                   BIT0
#define GENET_RX_SCB_BURST_SIZE                 (GENET_RX_BASE + 0x1040 + 0x0c)

#define GENET_TX_DMA_RING_CFG                   (GENET_TX_BASE + 0x1040 + 0x00)
#define GENET_TX_DMA_CTRL                       (GENET_TX_BASE + 0x1040 + 0x04)
#define  GENET_TX_DMA_CTRL_RBUF_EN(qid)         (BIT1 << (qid))
#define  GENET_TX_DMA_CTRL_EN                   BIT0
#define GENET_TX_SCB_BURST_SIZE                 (GENET_TX_BASE + 0x1040 + 0x0c)

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

  BCM_GENET_PLATFORM_DEVICE_PROTOCOL *Dev;

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

VOID
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

#endif /* GENET_UTIL_H__ */
