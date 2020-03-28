/** @file
  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause
**/

#include "Genet.h"
#include "SimpleNetwork.h"

#include <Library/DmaLib.h>

///
/// Simple Network Protocol instance
///
GLOBAL_REMOVE_IF_UNREFERENCED
EFI_SIMPLE_NETWORK_PROTOCOL gGenetSimpleNetwork = {
  EFI_SIMPLE_NETWORK_PROTOCOL_REVISION,       // Revision
  GenetSimpleNetworkStart,           // Start
  GenetSimpleNetworkStop,            // Stop
  GenetSimpleNetworkInitialize,      // Initialize 
  GenetSimpleNetworkReset,           // Reset
  GenetSimpleNetworkShutdown,        // Shutdown
  GenetSimpleNetworkReceiveFilters,  // ReceiveFilters
  GenetSimpleNetworkStationAddress,  // StationAddress 
  GenetSimpleNetworkStatistics,      // Statistics
  NULL,                                       // MCastIpToMac 
  GenetSimpleNetworkNvData,          // NvData
  GenetSimpleNetworkGetStatus,       // GetStatus
  GenetSimpleNetworkTransmit,        // Transmit
  GenetSimpleNetworkReceive,         // Receive
  NULL,                                       // WaitForPacket
  NULL                                        // Mode 
};

EFI_STATUS
EFIAPI
GenetSimpleNetworkStart (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This
  )
{
  GENET_PRIVATE_DATA *Genet;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  switch (Genet->SnpMode.State) {
    case EfiSimpleNetworkStarted:
    case EfiSimpleNetworkInitialized:
      return EFI_ALREADY_STARTED;
    default:
      Genet->SnpMode.State = EfiSimpleNetworkStarted;
      return EFI_SUCCESS;
  }
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkStop (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This
  )
{
  GENET_PRIVATE_DATA *Genet;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  GenetDisableTxRx (Genet);

  switch (Genet->SnpMode.State)
  {
    case EfiSimpleNetworkStarted:
    case EfiSimpleNetworkInitialized:
      Genet->SnpMode.State = EfiSimpleNetworkStopped;
      return EFI_SUCCESS;
    default:
      return EFI_DEVICE_ERROR;
  }
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkInitialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  IN UINTN                                    ExtraRxBufferSize,  OPTIONAL
  IN UINTN                                    ExtraTxBufferSize   OPTIONAL
  )
{
  GENET_PRIVATE_DATA *Genet;
  EFI_STATUS Status;
  UINTN n;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State == EfiSimpleNetworkInitialized) {
    return EFI_SUCCESS;
  }
  if (Genet->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  GenetReset (Genet);
  GenetSetPhyMode (Genet, Genet->PhyMode);

  Status = GenericPhyInit (&Genet->Phy);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GenetSetMacAddress (Genet, &Genet->SnpMode.CurrentAddress);
  GenetSetPromisc (Genet, TRUE);

  Status = GenericPhyUpdateConfig (&Genet->Phy);
  if (EFI_ERROR (Status)) {
    Genet->SnpMode.MediaPresent = FALSE;
  } else {
    Genet->SnpMode.MediaPresent = TRUE;
  }

  GenetDmaInitRings (Genet);

  // Map RX buffers
  for (n = 0; n < GENET_DMA_DESC_COUNT; n++) {
      Status = GenetDmaMapRxDescriptor (Genet, n);
      if (EFI_ERROR (Status)) {
          return Status;
      }
  }

  GenetEnableTxRx (Genet);

  Genet->SnpMode.State = EfiSimpleNetworkInitialized;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkReset (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  IN BOOLEAN                                  ExtendedVerification
  )
{
  GENET_PRIVATE_DATA *Genet;
  EFI_STATUS Status;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State == EfiSimpleNetworkStarted) {
    return EFI_DEVICE_ERROR;
  }
  if (Genet->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  Status = GenericPhyReset (&Genet->Phy);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkShutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This
  )
{
  GENET_PRIVATE_DATA *Genet;
  UINTN n;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State == EfiSimpleNetworkStarted) {
    return EFI_DEVICE_ERROR;
  }
  if (Genet->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  GenetDisableTxRx (Genet);

  for (n = 0; n < GENET_DMA_DESC_COUNT; n++) {
    GenetDmaUnmapRxDescriptor (Genet, n);
  }

  Genet->SnpMode.State = EfiSimpleNetworkStopped;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkReceiveFilters (
  IN EFI_SIMPLE_NETWORK_PROTOCOL               *This,
  IN UINT32                                    Enable,
  IN UINT32                                    Disable,
  IN BOOLEAN                                   ResetMCastFilter,
  IN UINTN                                     MCastFilterCnt,    OPTIONAL
  IN EFI_MAC_ADDRESS                           *MCastFilter       OPTIONAL
  )
{
  GENET_PRIVATE_DATA *Genet;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State == EfiSimpleNetworkStarted) {
    return EFI_DEVICE_ERROR;
  }
  if (Genet->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  // XXXTODO

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkStationAddress (
  IN EFI_SIMPLE_NETWORK_PROTOCOL               *This,
  IN BOOLEAN                                   Reset,
  IN EFI_MAC_ADDRESS                           *New    OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkStatistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL               *This,
  IN BOOLEAN                                   Reset,
  IN OUT UINTN                                 *StatisticsSize,   OPTIONAL
  OUT EFI_NETWORK_STATISTICS                   *StatisticsTable   OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkNvData (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  IN BOOLEAN                                  ReadWrite,  
  IN UINTN                                    Offset,
  IN UINTN                                    BufferSize,
  IN OUT VOID                                 *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkGetStatus (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  OUT UINT32                                  *InterruptStatus,  OPTIONAL
  OUT VOID                                    **TxBuf            OPTIONAL
  )
{
  GENET_PRIVATE_DATA *Genet;
  EFI_STATUS Status;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_NOT_STARTED;
  }

  Status = GenericPhyUpdateConfig (&Genet->Phy);
  if (EFI_ERROR (Status)) {
    Genet->SnpMode.MediaPresent = FALSE;
  } else {
    Genet->SnpMode.MediaPresent = TRUE;
  }

  if (TxBuf != NULL) {
    GenetTxIntr (Genet, TxBuf);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkTransmit (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  IN UINTN                                    HeaderSize,
  IN UINTN                                    BufferSize,
  IN VOID                                     *Buffer,
  IN EFI_MAC_ADDRESS                          *SrcAddr,   OPTIONAL
  IN EFI_MAC_ADDRESS                          *DestAddr,  OPTIONAL
  IN UINT16                                   *Protocol   OPTIONAL
  )
{
  GENET_PRIVATE_DATA *Genet;
  EFI_STATUS Status;
  UINT8 *Frame = Buffer;
  UINT8 Desc;
  PHYSICAL_ADDRESS DmaDeviceAddress;
  UINTN DmaNumberOfBytes;
  VOID *DmaMapping;

  if (This == NULL || Buffer == NULL) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkTransmit: Invalid parameter (missing handle or buffer)\n"));
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State != EfiSimpleNetworkInitialized) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkTransmit: Not started\n"));
    return EFI_NOT_STARTED;
  }

  if (HeaderSize != 0) {
    if (HeaderSize != Genet->SnpMode.MediaHeaderSize) {
      DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkTransmit: Invalid parameter (header size mismatch; HeaderSize 0x%X, SnpMode.MediaHeaderSize 0x%X))\n", HeaderSize, Genet->SnpMode.MediaHeaderSize));
      return EFI_INVALID_PARAMETER;
    }
    if (DestAddr == NULL || Protocol == NULL) {
      DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkTransmit: Invalid parameter (dest addr or protocol missing)\n"));
      return EFI_INVALID_PARAMETER;
    }
  }

  if (BufferSize < Genet->SnpMode.MediaHeaderSize) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkTransmit: Buffer too small\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  Status = EfiAcquireLockOrFail (&Genet->Lock);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkTransmit: Couldn't get lock: %r\n", Status));
    return EFI_ACCESS_DENIED;
  }

  if (Genet->TxQueued == GENET_DMA_DESC_COUNT - 1) {
    EfiReleaseLock (&Genet->Lock);

    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkTransmit: Queue full\n"));
    return EFI_NOT_READY;
  }

  if (HeaderSize != 0) {
    CopyMem (&Frame[0], &DestAddr->Addr[0], NET_ETHER_ADDR_LEN);
    CopyMem (&Frame[6], &SrcAddr->Addr[0], NET_ETHER_ADDR_LEN);
    Frame[12] = (*Protocol & 0xFF00) >> 8;
    Frame[13] = *Protocol & 0xFF;
  }

  Desc = Genet->TxProdIndex % GENET_DMA_DESC_COUNT;

  Genet->TxBuffer[Desc] = Frame;

  DmaNumberOfBytes = BufferSize;
  Status = DmaMap (MapOperationBusMasterRead,
                   (VOID *)(UINTN)Frame,
                   &DmaNumberOfBytes,
                   &DmaDeviceAddress,
                   &DmaMapping);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkTransmit: DmaMap failed: %r\n", Status));
    EfiReleaseLock (&Genet->Lock);
    return Status;
  }

  GenetDmaTriggerTx (Genet, Desc, DmaDeviceAddress, DmaNumberOfBytes);

  //DEBUG ((EFI_D_INFO, "GenetSimpleNetworkTransmit: Desc=%d VA=0x%X PA=0x%X Length=%d\n", Desc, Frame, DmaDeviceAddress, DmaNumberOfBytes));

  Genet->TxProdIndex = (Genet->TxProdIndex + 1) % 0xFFFF;
  Genet->TxQueued++;

  DmaUnmap (DmaMapping);

  EfiReleaseLock (&Genet->Lock);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetSimpleNetworkReceive (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL          *This,
  OUT    UINTN                                *HeaderSize, OPTIONAL
  IN OUT UINTN                                *BufferSize,
  OUT    VOID                                 *Buffer,
  OUT    EFI_MAC_ADDRESS                      *SrcAddr,    OPTIONAL
  OUT    EFI_MAC_ADDRESS                      *DestAddr,   OPTIONAL
  OUT    UINT16                               *Protocol    OPTIONAL
  )
{
  GENET_PRIVATE_DATA *Genet;
  EFI_STATUS Status;
  UINT8 DescIndex;
  UINT8 *Frame;
  UINTN FrameLength;

  if (This == NULL || Buffer == NULL) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkReceive: Invalid parameter (missing handle or buffer)\n"));
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State != EfiSimpleNetworkInitialized) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkReceive: Not started\n"));
    return EFI_NOT_STARTED;
  }

  Status = EfiAcquireLockOrFail (&Genet->Lock);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkReceive: Couldn't get lock: %r\n", Status));
    return EFI_ACCESS_DENIED;
  }
  
  Status = GenetRxIntr (Genet, &DescIndex, &FrameLength);
  if (EFI_ERROR (Status)) {
    EfiReleaseLock (&Genet->Lock);
    return Status;
  }

  ASSERT (Genet->RxBufferMap[DescIndex].Mapping != NULL);

  GenetDmaUnmapRxDescriptor (Genet, DescIndex);

  Frame = Genet->RxBuffer[DescIndex];

  if (FrameLength > 2 + Genet->SnpMode.MediaHeaderSize) {
    // Received frame has 2 bytes of padding at the start
    Frame += 2;
    FrameLength -= 2;

    if (*BufferSize < FrameLength) {
      DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkReceive: Buffer size (0x%X) is too small for frame (0x%X)\n", *BufferSize, FrameLength));
      Status = GenetDmaMapRxDescriptor (Genet, DescIndex);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkReceive: Failed to remap RX descriptor!\n"));
      }
      EfiReleaseLock (&Genet->Lock);
      return EFI_BUFFER_TOO_SMALL;
    }

    //DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkReceive: Frame [0x%X 0x%X]:", Genet->RxBuffer[DescIndex], Frame));
    //for (int i = 0; i < FrameLength; i++)
    //  DEBUG ((EFI_D_ERROR, " %02X", Frame[i]));
    //DEBUG ((EFI_D_ERROR, "\n"));

    if (DestAddr != NULL) {
      CopyMem (&DestAddr->Addr[0], &Frame[0], NET_ETHER_ADDR_LEN);
    }
    if (SrcAddr != NULL) {
      CopyMem (&SrcAddr->Addr[0], &Frame[6], NET_ETHER_ADDR_LEN);
    }
    if (Protocol != NULL) {
      *Protocol = (UINT16) ((Frame[12] << 8) | Frame[13]);
    }
    if (HeaderSize != NULL) {
      *HeaderSize = Genet->SnpMode.MediaHeaderSize;
    }

    CopyMem (Buffer, Frame, FrameLength);
    *BufferSize = FrameLength;
    
    Status = EFI_SUCCESS;
  } else {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkReceive: Short packet (FrameLength 0x%X)", FrameLength));
    Status = EFI_NOT_READY;
  }

  Status = GenetDmaMapRxDescriptor (Genet, DescIndex);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GenetSimpleNetworkReceive: Failed to remap RX descriptor!\n"));
  }

  EfiReleaseLock (&Genet->Lock);
  return Status;
}