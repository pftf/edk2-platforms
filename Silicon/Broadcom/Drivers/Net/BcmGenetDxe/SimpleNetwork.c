/** @file
  Provides the Simple Network functions.

  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "GenetUtil.h"

#include <Library/DmaLib.h>


/**
  Changes the state of a network interface from "stopped" to "started".

  @param  This Protocol instance pointer.

  @retval EFI_SUCCESS           The network interface was started.
  @retval EFI_ALREADY_STARTED   The network interface is already in the started state.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Changes the state of a network interface from "started" to "stopped".

  @param  This Protocol instance pointer.

  @retval EFI_SUCCESS           The network interface was stopped.
  @retval EFI_ALREADY_STARTED   The network interface is already in the stopped state.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Resets a network adapter and allocates the transmit and receive buffers
  required by the network interface; optionally, also requests allocation
  of additional transmit and receive buffers.

  @param  This              The protocol instance pointer.
  @param  ExtraRxBufferSize The size, in bytes, of the extra receive buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the extra
                            buffer, and the caller will not know if it is actually
                            being used.
  @param  ExtraTxBufferSize The size, in bytes, of the extra transmit buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the extra
                            buffer, and the caller will not know if it is actually
                            being used.

  @retval EFI_SUCCESS           The network interface was initialized.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_OUT_OF_RESOURCES  There was not enough memory for the transmit and
                                receive buffers.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Resets a network adapter and re-initializes it with the parameters that were
  provided in the previous call to Initialize().

  @param  This                 The protocol instance pointer.
  @param  ExtendedVerification Indicates that the driver may perform a more
                               exhaustive verification operation of the device
                               during reset.

  @retval EFI_SUCCESS           The network interface was reset.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Resets a network adapter and leaves it in a state that is safe for
  another driver to initialize.

  @param  This Protocol instance pointer.

  @retval EFI_SUCCESS           The network interface was shutdown.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Manages the multicast receive filters of a network interface.

  @param  This             The protocol instance pointer.
  @param  Enable           A bit mask of receive filters to enable on the network interface.
  @param  Disable          A bit mask of receive filters to disable on the network interface.
  @param  ResetMCastFilter Set to TRUE to reset the contents of the multicast receive
                           filters on the network interface to their default values.
  @param  McastFilterCnt   Number of multicast HW MAC addresses in the new
                           MCastFilter list. This value must be less than or equal to
                           the MCastFilterCnt field of EFI_SIMPLE_NETWORK_MODE. This
                           field is optional if ResetMCastFilter is TRUE.
  @param  MCastFilter      A pointer to a list of new multicast receive filter HW MAC
                           addresses. This list will replace any existing multicast
                           HW MAC address list. This field is optional if
                           ResetMCastFilter is TRUE.

  @retval EFI_SUCCESS           The multicast receive filter list was updated.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Modifies or resets the current station address, if supported.

  @param  This  The protocol instance pointer.
  @param  Reset Flag used to reset the station address to the network interfaces
                permanent address.
  @param  New   The new station address to be used for the network interface.

  @retval EFI_SUCCESS           The network interfaces station address was updated.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
EFI_STATUS
EFIAPI
GenetSimpleNetworkStationAddress (
  IN EFI_SIMPLE_NETWORK_PROTOCOL               *This,
  IN BOOLEAN                                   Reset,
  IN EFI_MAC_ADDRESS                           *New    OPTIONAL
  )
{
  GENET_PRIVATE_DATA *Genet;

  if (This == NULL || This->Mode == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Reset == TRUE && New == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State != EfiSimpleNetworkStarted) {
    return EFI_NOT_STARTED;
  }

  if (Reset) {
    // Use permanent address
    CopyMem (&This->Mode->CurrentAddress, &This->Mode->PermanentAddress, sizeof (This->Mode->CurrentAddress));
  } else {
    // Use specified address
    CopyMem (&This->Mode->CurrentAddress, New, sizeof (This->Mode->CurrentAddress));
  }

  GenetSetMacAddress (Genet, &Genet->SnpMode.CurrentAddress);

  return EFI_SUCCESS;
}

/**
  Resets or collects the statistics on a network interface.

  @param  This            Protocol instance pointer.
  @param  Reset           Set to TRUE to reset the statistics for the network interface.
  @param  StatisticsSize  On input the size, in bytes, of StatisticsTable. On
                          output the size, in bytes, of the resulting table of
                          statistics.
  @param  StatisticsTable A pointer to the EFI_NETWORK_STATISTICS structure that
                          contains the statistics.

  @retval EFI_SUCCESS           The statistics were collected from the network interface.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_BUFFER_TOO_SMALL  The Statistics buffer was too small. The current buffer
                                size needed to hold the statistics is returned in
                                StatisticsSize.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Performs read and write operations on the NVRAM device attached to a
  network interface.

  @param  This       The protocol instance pointer.
  @param  ReadWrite  TRUE for read operations, FALSE for write operations.
  @param  Offset     Byte offset in the NVRAM device at which to start the read or
                     write operation. This must be a multiple of NvRamAccessSize and
                     less than NvRamSize.
  @param  BufferSize The number of bytes to read or write from the NVRAM device.
                     This must also be a multiple of NvramAccessSize.
  @param  Buffer     A pointer to the data buffer.

  @retval EFI_SUCCESS           The NVRAM access was performed.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Reads the current interrupt status and recycled transmit buffer status from
  a network interface.

  @param  This            The protocol instance pointer.
  @param  InterruptStatus A pointer to the bit mask of the currently active interrupts
                          If this is NULL, the interrupt status will not be read from
                          the device. If this is not NULL, the interrupt status will
                          be read from the device. When the  interrupt status is read,
                          it will also be cleared. Clearing the transmit  interrupt
                          does not empty the recycled transmit buffer array.
  @param  TxBuf           Recycled transmit buffer address. The network interface will
                          not transmit if its internal recycled transmit buffer array
                          is full. Reading the transmit buffer does not clear the
                          transmit interrupt. If this is NULL, then the transmit buffer
                          status will not be read. If there are no transmit buffers to
                          recycle and TxBuf is not NULL, * TxBuf will be set to NULL.

  @retval EFI_SUCCESS           The status of the network interface was retrieved.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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

/**
  Places a packet in the transmit queue of a network interface.

  @param  This       The protocol instance pointer.
  @param  HeaderSize The size, in bytes, of the media header to be filled in by
                     the Transmit() function. If HeaderSize is non-zero, then it
                     must be equal to This->Mode->MediaHeaderSize and the DestAddr
                     and Protocol parameters must not be NULL.
  @param  BufferSize The size, in bytes, of the entire packet (media header and
                     data) to be transmitted through the network interface.
  @param  Buffer     A pointer to the packet (media header followed by data) to be
                     transmitted. This parameter cannot be NULL. If HeaderSize is zero,
                     then the media header in Buffer must already be filled in by the
                     caller. If HeaderSize is non-zero, then the media header will be
                     filled in by the Transmit() function.
  @param  SrcAddr    The source HW MAC address. If HeaderSize is zero, then this parameter
                     is ignored. If HeaderSize is non-zero and SrcAddr is NULL, then
                     This->Mode->CurrentAddress is used for the source HW MAC address.
  @param  DestAddr   The destination HW MAC address. If HeaderSize is zero, then this
                     parameter is ignored.
  @param  Protocol   The type of header to build. If HeaderSize is zero, then this
                     parameter is ignored. See RFC 1700, section "Ether Types", for
                     examples.

  @retval EFI_SUCCESS           The packet was placed on the transmit queue.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_NOT_READY         The network interface is too busy to accept this transmit request.
  @retval EFI_BUFFER_TOO_SMALL  The BufferSize parameter is too small.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkTransmit: Invalid parameter (missing handle or buffer)\n"));
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State != EfiSimpleNetworkInitialized) {
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkTransmit: Not started\n"));
    return EFI_NOT_STARTED;
  }

  if (HeaderSize != 0) {
    if (HeaderSize != Genet->SnpMode.MediaHeaderSize) {
      DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkTransmit: Invalid parameter (header size mismatch; HeaderSize 0x%X, SnpMode.MediaHeaderSize 0x%X))\n", HeaderSize, Genet->SnpMode.MediaHeaderSize));
      return EFI_INVALID_PARAMETER;
    }
    if (DestAddr == NULL || Protocol == NULL) {
      DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkTransmit: Invalid parameter (dest addr or protocol missing)\n"));
      return EFI_INVALID_PARAMETER;
    }
  }

  if (BufferSize < Genet->SnpMode.MediaHeaderSize) {
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkTransmit: Buffer too small\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  Status = EfiAcquireLockOrFail (&Genet->Lock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkTransmit: Couldn't get lock: %r\n", Status));
    return EFI_ACCESS_DENIED;
  }

  if (Genet->TxQueued == GENET_DMA_DESC_COUNT - 1) {
    EfiReleaseLock (&Genet->Lock);

    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkTransmit: Queue full\n"));
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
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkTransmit: DmaMap failed: %r\n", Status));
    EfiReleaseLock (&Genet->Lock);
    return Status;
  }

  GenetDmaTriggerTx (Genet, Desc, DmaDeviceAddress, DmaNumberOfBytes);

  //DEBUG ((DEBUG_INFO, "GenetSimpleNetworkTransmit: Desc=%d VA=0x%X PA=0x%X Length=%d\n", Desc, Frame, DmaDeviceAddress, DmaNumberOfBytes));

  Genet->TxProdIndex = (Genet->TxProdIndex + 1) % 0xFFFF;
  Genet->TxQueued++;

  DmaUnmap (DmaMapping);

  EfiReleaseLock (&Genet->Lock);

  return EFI_SUCCESS;
}

/**
  Receives a packet from a network interface.

  @param  This       The protocol instance pointer.
  @param  HeaderSize The size, in bytes, of the media header received on the network
                     interface. If this parameter is NULL, then the media header size
                     will not be returned.
  @param  BufferSize On entry, the size, in bytes, of Buffer. On exit, the size, in
                     bytes, of the packet that was received on the network interface.
  @param  Buffer     A pointer to the data buffer to receive both the media header and
                     the data.
  @param  SrcAddr    The source HW MAC address. If this parameter is NULL, the
                     HW MAC source address will not be extracted from the media
                     header.
  @param  DestAddr   The destination HW MAC address. If this parameter is NULL,
                     the HW MAC destination address will not be extracted from the
                     media header.
  @param  Protocol   The media header type. If this parameter is NULL, then the
                     protocol will not be extracted from the media header. See
                     RFC 1700 section "Ether Types" for examples.

  @retval  EFI_SUCCESS           The received data was stored in Buffer, and BufferSize has
                                 been updated to the number of bytes received.
  @retval  EFI_NOT_STARTED       The network interface has not been started.
  @retval  EFI_NOT_READY         The network interface is too busy to accept this transmit
                                 request.
  @retval  EFI_BUFFER_TOO_SMALL  The BufferSize parameter is too small.
  @retval  EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval  EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval  EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
STATIC
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
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkReceive: Invalid parameter (missing handle or buffer)\n"));
    return EFI_INVALID_PARAMETER;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);
  if (Genet->SnpMode.State != EfiSimpleNetworkInitialized) {
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkReceive: Not started\n"));
    return EFI_NOT_STARTED;
  }

  Status = EfiAcquireLockOrFail (&Genet->Lock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkReceive: Couldn't get lock: %r\n", Status));
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
      DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkReceive: Buffer size (0x%X) is too small for frame (0x%X)\n", *BufferSize, FrameLength));
      Status = GenetDmaMapRxDescriptor (Genet, DescIndex);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkReceive: Failed to remap RX descriptor!\n"));
      }
      EfiReleaseLock (&Genet->Lock);
      return EFI_BUFFER_TOO_SMALL;
    }

    //DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkReceive: Frame [0x%X 0x%X]:", Genet->RxBuffer[DescIndex], Frame));
    //for (int i = 0; i < FrameLength; i++)
    //  DEBUG ((DEBUG_ERROR, " %02X", Frame[i]));
    //DEBUG ((DEBUG_ERROR, "\n"));

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
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkReceive: Short packet (FrameLength 0x%X)", FrameLength));
    Status = EFI_NOT_READY;
  }

  Status = GenetDmaMapRxDescriptor (Genet, DescIndex);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "GenetSimpleNetworkReceive: Failed to remap RX descriptor!\n"));
  }

  EfiReleaseLock (&Genet->Lock);
  return Status;
}


/**
  This function converts a multicast IP address to a multicast HW MAC address
  for all packet transactions.

  @param [in] pSimpleNetwork    Protocol instance pointer
  @param [in] bIPv6             Set to TRUE if the multicast IP address is IPv6 [RFC2460].
                                Set to FALSE if the multicast IP address is IPv4 [RFC 791].
  @param [in] pIP               The multicast IP address that is to be converted to a
                                multicast HW MAC address.
  @param [in] pMAC              The multicast HW MAC address that is to be generated from IP.

  @retval EFI_SUCCESS           This operation was successful.
  @retval EFI_NOT_STARTED       The network interface was not started.
  @retval EFI_INVALID_PARAMETER pSimpleNetwork parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       The increased buffer size feature is not supported.

**/
STATIC
EFI_STATUS
EFIAPI
GenetSimpleNetworkMCastIPtoMAC (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * pSimpleNetwork,
  IN BOOLEAN bIPv6,
  IN EFI_IP_ADDRESS * pIP,
  OUT EFI_MAC_ADDRESS * pMAC
  )
{
  return EFI_UNSUPPORTED;
}

///
/// Simple Network Protocol instance
///
GLOBAL_REMOVE_IF_UNREFERENCED
EFI_SIMPLE_NETWORK_PROTOCOL gGenetSimpleNetwork = {
  EFI_SIMPLE_NETWORK_PROTOCOL_REVISION,       // Revision
  GenetSimpleNetworkStart,                    // Start
  GenetSimpleNetworkStop,                     // Stop
  GenetSimpleNetworkInitialize,               // Initialize
  GenetSimpleNetworkReset,                    // Reset
  GenetSimpleNetworkShutdown,                 // Shutdown
  GenetSimpleNetworkReceiveFilters,           // ReceiveFilters
  GenetSimpleNetworkStationAddress,           // StationAddress
  GenetSimpleNetworkStatistics,               // Statistics
  GenetSimpleNetworkMCastIPtoMAC,             // MCastIpToMac
  GenetSimpleNetworkNvData,                   // NvData
  GenetSimpleNetworkGetStatus,                // GetStatus
  GenetSimpleNetworkTransmit,                 // Transmit
  GenetSimpleNetworkReceive,                  // Receive
  NULL,                                       // WaitForPacket
  NULL                                        // Mode
};