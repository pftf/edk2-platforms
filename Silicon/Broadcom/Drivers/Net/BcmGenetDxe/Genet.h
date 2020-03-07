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

  UINTN                           RegBase;
} GENET_PRIVATE_DATA;

#define GENET_DRIVER_SIGNATURE                SIGNATURE_32('G', 'N', 'E', 'T')
#define GENET_PRIVATE_DATA_FROM_SNP_THIS(a)   CR(a, GENET_PRIVATE_DATA, Snp, GENET_DRIVER_SIGNATURE)

EFI_STATUS
EFIAPI
GenetDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
  );

EFI_STATUS
EFIAPI
GenetDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
  );

EFI_STATUS
EFIAPI
GenetDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer   OPTIONAL
  );

#endif /* BCM_GENET_H__ */
