/** @file
  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause
**/

#include "Genet.h"
#include "GenetReg.h"
#include "SimpleNetwork.h"

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
  GenetSimpleNetworkMCastIpToMac,    // MCastIpToMac 
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

  // XXXTODO
  //GenetStop (Genet);

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

  // XXXTODO

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
GenetSimpleNetworkShutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This
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
GenetSimpleNetworkMCastIpToMac (
  IN EFI_SIMPLE_NETWORK_PROTOCOL               *This,
  IN BOOLEAN                                   IPv6,
  IN EFI_IP_ADDRESS                            *IP,
  OUT EFI_MAC_ADDRESS                          *MAC
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
  return EFI_UNSUPPORTED;
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
  return EFI_UNSUPPORTED;
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
  return EFI_NOT_READY;
}