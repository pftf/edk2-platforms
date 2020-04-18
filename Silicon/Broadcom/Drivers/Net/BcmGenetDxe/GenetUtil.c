/** @file

  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "GenetUtil.h"

#include <Library/ArmLib.h>
#include <Library/DmaLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define __LOWEST_SET_BIT(__mask)    ((((__mask) - 1) & (__mask)) ^ (__mask))
#define __SHIFTOUT(__x, __mask)     (((__x) & (__mask)) / __LOWEST_SET_BIT(__mask))
#define __SHIFTIN(__x, __mask)      ((__x) * __LOWEST_SET_BIT(__mask))

#define GENET_PHY_RETRY     1000

UINT32
EFIAPI
GenetMmioRead (
    IN GENET_PRIVATE_DATA   *Genet,
    IN UINT32               Offset
    )
{
    ASSERT((Offset & 3) == 0);

    return MmioRead32 (Genet->RegBase + Offset);
}

VOID
EFIAPI
GenetMmioWrite (
    IN GENET_PRIVATE_DATA   *Genet,
    IN UINT32               Offset,
    IN UINT32               Data
    )
{
    ASSERT((Offset & 3) == 0);

    MmioWrite32 (Genet->RegBase + Offset, Data);
    ArmDataMemoryBarrier ();
}

EFI_STATUS
EFIAPI
GenetPhyRead (
    IN VOID                 *Priv,
    IN UINT8                PhyAddr,
    IN UINT8                Reg,
    OUT UINT16              *Data
    )
{
    GENET_PRIVATE_DATA *Genet = Priv;
    UINTN Retry;
    UINT32 Value;

    Value = GENET_MDIO_READ |
            GENET_MDIO_START_BUSY |
            __SHIFTIN(PhyAddr, GENET_MDIO_PMD) |
            __SHIFTIN(Reg, GENET_MDIO_REG);
    GenetMmioWrite (Genet, GENET_MDIO_CMD, Value);

    for (Retry = GENET_PHY_RETRY; Retry > 0; Retry--) {
        Value = GenetMmioRead (Genet, GENET_MDIO_CMD);
        if ((Value & GENET_MDIO_START_BUSY) == 0) {
            *Data = Value & 0xffff;
            break;
        }
        gBS->Stall (10);
    }

    if (Retry == 0) {
        DEBUG ((DEBUG_ERROR, "GenetPhyRead: Timeout reading PhyAddr %d, Reg %d\n", PhyAddr, Reg));
        return EFI_TIMEOUT;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetPhyWrite (
    IN VOID                 *Priv,
    IN UINT8                PhyAddr,
    IN UINT8                Reg,
    IN UINT16               Data
    )
{
    GENET_PRIVATE_DATA *Genet = Priv;
    UINTN Retry;
    UINT32 Value;

    Value = GENET_MDIO_WRITE |
            GENET_MDIO_START_BUSY |
            __SHIFTIN(PhyAddr, GENET_MDIO_PMD) |
            __SHIFTIN(Reg, GENET_MDIO_REG);
    GenetMmioWrite (Genet, GENET_MDIO_CMD, Value | Data);

    for (Retry = GENET_PHY_RETRY; Retry > 0; Retry--) {
        Value = GenetMmioRead (Genet, GENET_MDIO_CMD);
        if ((Value & GENET_MDIO_START_BUSY) == 0) {
            break;
        }
        gBS->Stall (10);
    }

    if (Retry == 0) {
        DEBUG ((DEBUG_ERROR, "GenetPhyRead: Timeout writing PhyAddr %d, Reg %d\n", PhyAddr, Reg));
        return EFI_TIMEOUT;
    }

    return EFI_SUCCESS;
}

VOID
EFIAPI
GenetPhyConfigure (
    IN VOID                 *Priv,
    IN GENERIC_PHY_SPEED    Speed,
    IN GENERIC_PHY_DUPLEX   Duplex
    )
{
    GENET_PRIVATE_DATA *Genet = Priv;
    UINT32 Value;

    Value = GenetMmioRead (Genet, GENET_EXT_RGMII_OOB_CTRL);
    Value &= ~GENET_EXT_RGMII_OOB_OOB_DISABLE;
    Value |= GENET_EXT_RGMII_OOB_RGMII_LINK;
    Value |= GENET_EXT_RGMII_OOB_RGMII_MODE_EN;
    if (Genet->PhyMode == GENET_PHY_MODE_RGMII)
        Value |= GENET_EXT_RGMII_OOB_ID_MODE_DISABLE;
    GenetMmioWrite (Genet, GENET_EXT_RGMII_OOB_CTRL, Value);

    Value = GenetMmioRead (Genet, GENET_UMAC_CMD);
    Value &= ~GENET_UMAC_CMD_SPEED;
    switch (Speed) {
        case PHY_SPEED_1000:
            Value |= __SHIFTIN(GENET_UMAC_CMD_SPEED_1000, GENET_UMAC_CMD_SPEED);
            break;
        case PHY_SPEED_100:
            Value |= __SHIFTIN(GENET_UMAC_CMD_SPEED_100, GENET_UMAC_CMD_SPEED);
            break;
        default:
            Value |= __SHIFTIN(GENET_UMAC_CMD_SPEED_10, GENET_UMAC_CMD_SPEED);
            break;
    }
    if (Duplex == PHY_DUPLEX_FULL) {
        Value &= ~GENET_UMAC_CMD_HD_EN;
    } else {
        Value |= GENET_UMAC_CMD_HD_EN;
    }
    GenetMmioWrite (Genet, GENET_UMAC_CMD, Value);
}

VOID
EFIAPI
GenetReset (
    IN GENET_PRIVATE_DATA * Genet
    )
{
    UINT32 Value;

    Value = GenetMmioRead (Genet, GENET_SYS_RBUF_FLUSH_CTRL);
    Value |= GENET_SYS_RBUF_FLUSH_RESET;
    GenetMmioWrite (Genet, GENET_SYS_RBUF_FLUSH_CTRL, Value);
    gBS->Stall (10);

    Value &= ~GENET_SYS_RBUF_FLUSH_RESET;
    GenetMmioWrite (Genet, GENET_SYS_RBUF_FLUSH_CTRL, Value);
    gBS->Stall (10);

    GenetMmioWrite (Genet, GENET_SYS_RBUF_FLUSH_CTRL, 0);
    gBS->Stall (10);

    GenetMmioWrite (Genet, GENET_UMAC_CMD, 0);
    GenetMmioWrite (Genet, GENET_UMAC_CMD, GENET_UMAC_CMD_LCL_LOOP_EN | GENET_UMAC_CMD_SW_RESET);
    gBS->Stall (10);
    GenetMmioWrite (Genet, GENET_UMAC_CMD, 0);

    GenetMmioWrite (Genet, GENET_UMAC_MIB_CTRL, GENET_UMAC_MIB_RESET_RUNT | GENET_UMAC_MIB_RESET_RX | GENET_UMAC_MIB_RESET_TX);
    GenetMmioWrite (Genet, GENET_UMAC_MIB_CTRL, 0);

    GenetMmioWrite (Genet, GENET_UMAC_MAX_FRAME_LEN, 1536);

    Value = GenetMmioRead (Genet, GENET_RBUF_CTRL);
    Value |= GENET_RBUF_ALIGN_2B;
    GenetMmioWrite (Genet, GENET_RBUF_CTRL, Value);

    GenetMmioWrite (Genet, GENET_RBUF_TBUF_SIZE_CTRL, 1);
}

VOID
EFIAPI
GenetSetMacAddress (
    IN GENET_PRIVATE_DATA * Genet,
    IN EFI_MAC_ADDRESS *    MacAddr
    )
{
    UINT32 Value;

    Value = MacAddr->Addr[3] |
            MacAddr->Addr[2] << 8 |
            MacAddr->Addr[1] << 16 |
            MacAddr->Addr[0] << 24;
    GenetMmioWrite (Genet, GENET_UMAC_MAC0, Value);
    Value = MacAddr->Addr[5] |
            MacAddr->Addr[4] << 8;
    GenetMmioWrite (Genet, GENET_UMAC_MAC1, Value);
}

VOID
EFIAPI
GenetSetPhyMode (
    IN GENET_PRIVATE_DATA * Genet,
    IN GENET_PHY_MODE       PhyMode
    )
{
    UINT32 Value;

    switch (PhyMode) {
        case GENET_PHY_MODE_RGMII:
        case GENET_PHY_MODE_RGMII_RXID:
            Value = GENET_SYS_PORT_MODE_EXT_GPHY;
            break;
        default:
            Value = 0;
            break;
    }
    GenetMmioWrite (Genet, GENET_SYS_PORT_CTRL, Value);
}

VOID
EFIAPI
GenetEnableTxRx (
    IN GENET_PRIVATE_DATA * Genet
    )
{
    UINT32 Value;
    UINT8 Qid = GENET_DMA_DEFAULT_QUEUE;

    // Start TX DMA on default queue
    Value = GenetMmioRead (Genet, GENET_TX_DMA_CTRL);
    Value |= GENET_TX_DMA_CTRL_EN;
    Value |= GENET_TX_DMA_CTRL_RBUF_EN(Qid);
    GenetMmioWrite (Genet, GENET_TX_DMA_CTRL, Value);

    // Start RX DMA on default queue
    Value = GenetMmioRead (Genet, GENET_RX_DMA_CTRL);
    Value |= GENET_RX_DMA_CTRL_EN;
    Value |= GENET_RX_DMA_CTRL_RBUF_EN(Qid);
    GenetMmioWrite (Genet, GENET_RX_DMA_CTRL, Value);

    // Enable transmitter and receiver
    Value = GenetMmioRead (Genet, GENET_UMAC_CMD);
    Value |= GENET_UMAC_CMD_TXEN | GENET_UMAC_CMD_RXEN;
    GenetMmioWrite (Genet, GENET_UMAC_CMD, Value);

    // Enable interrupts
    GenetMmioWrite (Genet, GENET_INTRL2_CPU_CLEAR_MASK, GENET_IRQ_TXDMA_DONE | GENET_IRQ_RXDMA_DONE);
}

VOID
EFIAPI
GenetDisableTxRx (
    IN GENET_PRIVATE_DATA * Genet
    )
{
    UINT32 Value;

    // Disable interrupts
    GenetMmioWrite (Genet, GENET_INTRL2_CPU_SET_MASK, 0xFFFFFFFF);
    GenetMmioWrite (Genet, GENET_INTRL2_CPU_CLEAR, 0xFFFFFFFF);

    // Disable receiver
    Value = GenetMmioRead (Genet, GENET_UMAC_CMD);
    Value &= ~GENET_UMAC_CMD_RXEN;
    GenetMmioWrite (Genet, GENET_UMAC_CMD, Value);

    // Stop RX DMA
    Value = GenetMmioRead (Genet, GENET_RX_DMA_CTRL);
    Value &= ~GENET_RX_DMA_CTRL_EN;
    GenetMmioWrite (Genet, GENET_RX_DMA_CTRL, Value);

    // Stop TX DMA
    Value = GenetMmioRead (Genet, GENET_TX_DMA_CTRL);
    Value &= ~GENET_TX_DMA_CTRL_EN;
    GenetMmioWrite (Genet, GENET_TX_DMA_CTRL, Value);

    // Flush data in the TX FIFO
    GenetMmioWrite (Genet, GENET_UMAC_TX_FLUSH, 1);
    gBS->Stall (10);
    GenetMmioWrite (Genet, GENET_UMAC_TX_FLUSH, 0);

    // Disable transmitter
    Value = GenetMmioRead (Genet, GENET_UMAC_CMD);
    Value &= ~GENET_UMAC_CMD_TXEN;
    GenetMmioWrite (Genet, GENET_UMAC_CMD, Value);
}

VOID
EFIAPI
GenetSetPromisc (
    IN GENET_PRIVATE_DATA * Genet,
    IN BOOLEAN              Enable
    )
{
    UINT32 Value;

    Value = GenetMmioRead (Genet, GENET_UMAC_CMD);
    if (Enable) {
        Value |= GENET_UMAC_CMD_PROMISC;
    } else {
        Value &= ~GENET_UMAC_CMD_PROMISC;
    }
    GenetMmioWrite (Genet, GENET_UMAC_CMD, Value);
}

VOID
EFIAPI
GenetDmaInitRings (
    IN GENET_PRIVATE_DATA * Genet
    )
{
    UINT8 Qid = GENET_DMA_DEFAULT_QUEUE;

    Genet->TxQueued = 0;
    Genet->TxNext = 0;
    Genet->TxConsIndex = 0;
    Genet->TxProdIndex = 0;

    Genet->RxConsIndex = 0;
    Genet->RxProdIndex = 0;

    // Configure TX queue
    GenetMmioWrite (Genet, GENET_TX_SCB_BURST_SIZE, 0x08);
    GenetMmioWrite (Genet, GENET_TX_DMA_READ_PTR_LO(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_READ_PTR_HI(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_CONS_INDEX(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_PROD_INDEX(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_RING_BUF_SIZE(Qid),
                    __SHIFTIN(GENET_DMA_DESC_COUNT, GENET_TX_DMA_RING_BUF_SIZE_DESC_COUNT) |
                    __SHIFTIN(GENET_MAX_PACKET_SIZE, GENET_TX_DMA_RING_BUF_SIZE_BUF_LENGTH));
    GenetMmioWrite (Genet, GENET_TX_DMA_START_ADDR_LO(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_START_ADDR_HI(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_END_ADDR_LO(Qid),
                    GENET_DMA_DESC_COUNT * GENET_DMA_DESC_SIZE / 4 - 1);
    GenetMmioWrite (Genet, GENET_TX_DMA_END_ADDR_HI(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_MBUF_DONE_THRES(Qid), 1);
    GenetMmioWrite (Genet, GENET_TX_DMA_FLOW_PERIOD(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_WRITE_PTR_LO(Qid), 0);
    GenetMmioWrite (Genet, GENET_TX_DMA_WRITE_PTR_HI(Qid), 0);

    // Enable TX queue
    GenetMmioWrite (Genet, GENET_TX_DMA_RING_CFG, (1U << Qid));

    // Configure RX queue
    GenetMmioWrite (Genet, GENET_RX_SCB_BURST_SIZE, 0x08);
    GenetMmioWrite (Genet, GENET_RX_DMA_WRITE_PTR_LO(Qid), 0);
    GenetMmioWrite (Genet, GENET_RX_DMA_WRITE_PTR_HI(Qid), 0);
    GenetMmioWrite (Genet, GENET_RX_DMA_PROD_INDEX(Qid), 0);
    GenetMmioWrite (Genet, GENET_RX_DMA_CONS_INDEX(Qid), 0);
    GenetMmioWrite (Genet, GENET_RX_DMA_RING_BUF_SIZE(Qid),
                    __SHIFTIN(GENET_DMA_DESC_COUNT, GENET_RX_DMA_RING_BUF_SIZE_DESC_COUNT) |
                    __SHIFTIN(GENET_MAX_PACKET_SIZE, GENET_RX_DMA_RING_BUF_SIZE_BUF_LENGTH));
    GenetMmioWrite (Genet, GENET_RX_DMA_START_ADDR_LO(Qid), 0);
    GenetMmioWrite (Genet, GENET_RX_DMA_START_ADDR_HI(Qid), 0);
    GenetMmioWrite (Genet, GENET_RX_DMA_END_ADDR_LO(Qid),
                    GENET_DMA_DESC_COUNT * GENET_DMA_DESC_SIZE / 4 - 1);
    GenetMmioWrite (Genet, GENET_RX_DMA_END_ADDR_HI(Qid), 0);
    GenetMmioWrite (Genet, GENET_RX_DMA_XON_XOFF_THRES(Qid),
                    __SHIFTIN(5, GENET_RX_DMA_XON_XOFF_THRES_LO) |
                    __SHIFTIN(GENET_DMA_DESC_COUNT >> 4, GENET_RX_DMA_XON_XOFF_THRES_HI));
    GenetMmioWrite (Genet, GENET_RX_DMA_READ_PTR_LO(Qid), 0);
    GenetMmioWrite (Genet, GENET_RX_DMA_READ_PTR_HI(Qid), 0);

    // Enable RX queue
    GenetMmioWrite (Genet, GENET_RX_DMA_RING_CFG, (1U << Qid));
}

EFI_STATUS
EFIAPI
GenetDmaAlloc (
    IN GENET_PRIVATE_DATA * Genet
    )
{
    EFI_STATUS Status;
    UINTN n;

    for (n = 0; n < GENET_DMA_DESC_COUNT; n++) {
        Status = DmaAllocateBuffer (EfiBootServicesData, EFI_SIZE_TO_PAGES (GENET_MAX_PACKET_SIZE), (VOID **)&Genet->RxBuffer[n]);
        if (EFI_ERROR (Status)) {
            DEBUG ((DEBUG_ERROR, "GenetDmaAlloc: Failed to allocate RX buffer: %r\n", Status));
            GenetDmaFree (Genet);
            return Status;
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetDmaMapRxDescriptor (
    IN GENET_PRIVATE_DATA * Genet,
    IN UINT8                DescIndex
    )
{
    EFI_STATUS Status;
    UINTN DmaNumberOfBytes;

    ASSERT (Genet->RxBufferMap[DescIndex].Mapping == NULL);
    ASSERT (Genet->RxBuffer[DescIndex] != NULL);

    DmaNumberOfBytes = GENET_MAX_PACKET_SIZE;
    Status = DmaMap (MapOperationBusMasterWrite,
                (VOID *)Genet->RxBuffer[DescIndex],
                &DmaNumberOfBytes,
                &Genet->RxBufferMap[DescIndex].Pa,
                &Genet->RxBufferMap[DescIndex].Mapping);
    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "GenetDmaMapRxDescriptor: Failed to map RX buffer: %r\n", Status));
        return Status;
    }

    //DEBUG ((DEBUG_INFO, "GenetDmaMapRxDescriptor: Desc 0x%X mapped to 0x%X\n", DescIndex, Genet->RxBufferMap[DescIndex].Pa));

    GenetMmioWrite (Genet, GENET_RX_DESC_ADDRESS_LO (DescIndex), Genet->RxBufferMap[DescIndex].Pa & 0xFFFFFFFF);
    GenetMmioWrite (Genet, GENET_RX_DESC_ADDRESS_HI (DescIndex), (Genet->RxBufferMap[DescIndex].Pa >> 32) & 0xFFFFFFFF);

    return EFI_SUCCESS;
}

VOID
EFIAPI
GenetDmaUnmapRxDescriptor (
    IN GENET_PRIVATE_DATA * Genet,
    IN UINT8                DescIndex
    )
{
    if (Genet->RxBufferMap[DescIndex].Mapping != NULL) {
        DmaUnmap (Genet->RxBufferMap[DescIndex].Mapping);
        Genet->RxBufferMap[DescIndex].Mapping = NULL;
    }
}

VOID
EFIAPI
GenetDmaFree (
    IN GENET_PRIVATE_DATA * Genet
    )
{
    UINTN n;

    for (n = 0; n < GENET_DMA_DESC_COUNT; n++) {
        GenetDmaUnmapRxDescriptor (Genet, n);

        if (Genet->RxBuffer[n] != NULL) {
            DmaFreeBuffer (EFI_SIZE_TO_PAGES (GENET_MAX_PACKET_SIZE), Genet->RxBuffer[n]);
            Genet->RxBuffer[n] = NULL;
        }
    }
}

VOID
EFIAPI
GenetDmaTriggerTx (
    IN GENET_PRIVATE_DATA * Genet,
    IN UINT8                DescIndex,
    IN EFI_PHYSICAL_ADDRESS PhysAddr,
    IN UINTN                NumberOfBytes
    )
{
    UINT32 DescStatus;
    UINT8 Qid = GENET_DMA_DEFAULT_QUEUE;

    DescStatus = GENET_TX_DESC_STATUS_SOP |
                 GENET_TX_DESC_STATUS_EOP |
                 GENET_TX_DESC_STATUS_CRC |
                 GENET_TX_DESC_STATUS_QTAG |
                 __SHIFTIN(NumberOfBytes, GENET_TX_DESC_STATUS_BUFLEN);

    GenetMmioWrite (Genet, GENET_TX_DESC_ADDRESS_LO(DescIndex), PhysAddr & 0xFFFFFFFF);
    GenetMmioWrite (Genet, GENET_TX_DESC_ADDRESS_HI(DescIndex), (PhysAddr >> 32) & 0xFFFFFFFF);
    GenetMmioWrite (Genet, GENET_TX_DESC_STATUS(DescIndex), DescStatus);

    GenetMmioWrite (Genet, GENET_TX_DMA_PROD_INDEX (Qid), (DescIndex + 1) & 0xFFFF);
}

VOID
EFIAPI
GenetTxIntr (
    IN GENET_PRIVATE_DATA * Genet,
    OUT VOID **             TxBuf
    )
{
    UINT32 ConsIndex, Total;
    UINT8 Qid = GENET_DMA_DEFAULT_QUEUE;

    ConsIndex = GenetMmioRead (Genet, GENET_TX_DMA_CONS_INDEX (Qid)) & 0xFFFF;
    Total = (ConsIndex - Genet->TxConsIndex) & 0xFFFF;
    if (Genet->TxQueued > 0 && Total > 0) {
        *TxBuf = Genet->TxBuffer[Genet->TxNext];
        Genet->TxQueued--;
        Genet->TxNext = (Genet->TxNext + 1) % GENET_DMA_DESC_COUNT;
        Genet->TxConsIndex++;
    } else {
        *TxBuf = NULL;
    }
}

EFI_STATUS
EFIAPI
GenetRxIntr (
    IN GENET_PRIVATE_DATA * Genet,
    OUT UINT8 *             DescIndex,
    OUT UINTN *             FrameLength
    )
{
    EFI_STATUS Status;
    UINT32 ProdIndex, Total;
    UINT32 DescStatus;
    UINT8 Qid = GENET_DMA_DEFAULT_QUEUE;

    ProdIndex = GenetMmioRead (Genet, GENET_RX_DMA_PROD_INDEX (Qid)) & 0xFFFF;
    Total = (ProdIndex - Genet->RxConsIndex) & 0xFFFF;
    if (Total > 0) {
        *DescIndex = Genet->RxConsIndex % GENET_DMA_DESC_COUNT;
        DescStatus = GenetMmioRead (Genet, GENET_RX_DESC_STATUS (*DescIndex));
        *FrameLength = __SHIFTOUT (DescStatus, GENET_RX_DESC_STATUS_BUFLEN);

        //DEBUG ((DEBUG_INFO, "GenetRxIntr: DescIndex=0x%X FrameLength=0x%X\n", *DescIndex, *FrameLength));

        Genet->RxConsIndex = (Genet->RxConsIndex + 1) & 0xFFFF;
        GenetMmioWrite (Genet, GENET_RX_DMA_CONS_INDEX (Qid), Genet->RxConsIndex);
        Status = EFI_SUCCESS;
    } else {
        Status = EFI_NOT_READY;
    }

    return Status;
}
