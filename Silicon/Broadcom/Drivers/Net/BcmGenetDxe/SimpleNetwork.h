/** @file
  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef GENET_SIMPLENETWORK_H__
#define GENET_SIMPLENETWORK_H__

extern EFI_SIMPLE_NETWORK_PROTOCOL gGenetSimpleNetwork;

EFI_STATUS
EFIAPI
GenetSimpleNetworkStart (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkStop (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkInitialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  IN UINTN                                    ExtraRxBufferSize,  OPTIONAL
  IN UINTN                                    ExtraTxBufferSize   OPTIONAL
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkReset (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  IN BOOLEAN                                  ExtendedVerification
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkShutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkReceiveFilters (
  IN EFI_SIMPLE_NETWORK_PROTOCOL               *This,
  IN UINT32                                    Enable,
  IN UINT32                                    Disable,
  IN BOOLEAN                                   ResetMCastFilter,
  IN UINTN                                     MCastFilterCnt,    OPTIONAL
  IN EFI_MAC_ADDRESS                           *MCastFilter       OPTIONAL
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkStationAddress (
  IN EFI_SIMPLE_NETWORK_PROTOCOL               *This,
  IN BOOLEAN                                   Reset,
  IN EFI_MAC_ADDRESS                           *New    OPTIONAL
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkStatistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL               *This,
  IN BOOLEAN                                   Reset,
  IN OUT UINTN                                 *StatisticsSize,   OPTIONAL
  OUT EFI_NETWORK_STATISTICS                   *StatisticsTable   OPTIONAL
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkNvData (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  IN BOOLEAN                                  ReadWrite,
  IN UINTN                                    Offset,
  IN UINTN                                    BufferSize,
  IN OUT VOID                                 *Buffer
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkGetStatus (
  IN EFI_SIMPLE_NETWORK_PROTOCOL              *This,
  OUT UINT32                                  *InterruptStatus,  OPTIONAL
  OUT VOID                                    **TxBuf            OPTIONAL
  );

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
  );

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
  );

EFI_STATUS
EFIAPI
GenetSimpleNetworkMCastIPtoMAC (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * pSimpleNetwork,
  IN BOOLEAN bIPv6,
  IN EFI_IP_ADDRESS * pIP,
  OUT EFI_MAC_ADDRESS * pMAC
  );

#endif // GENET_SIMPLENETWORK_H__
