
/** @file
  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "Genet.h"
#include "SimpleNetwork.h"

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

GLOBAL_REMOVE_IF_UNREFERENCED EFI_DRIVER_BINDING_PROTOCOL gGenetDriverBinding = {
  GenetDriverBindingSupported,
  GenetDriverBindingStart,
  GenetDriverBindingStop,
  GENET_VERSION,
  NULL,
  NULL
};

STATIC EFI_HANDLE mDevice;

STATIC GENET_DEVICE_PATH mDevicePath = {
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_MAC_ADDR_DP,
      {
        (UINT8)(sizeof (MAC_ADDR_DEVICE_PATH)),
        (UINT8)((sizeof (MAC_ADDR_DEVICE_PATH)) >> 8)
      }
    },
    {{ 0 }},
    NET_IFTYPE_ETHERNET
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      0
    }
  }
};

EFI_STATUS
EFIAPI
GenetDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
  )
{
  VOID *Dev;
  EFI_STATUS Status;

  if (ControllerHandle != mDevice) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->HandleProtocol (ControllerHandle,
                                &gEfiSimpleNetworkProtocolGuid,
                                (VOID **)&Dev
                                );
  if (Status == EFI_SUCCESS) {
    return EFI_ALREADY_STARTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenetDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
  )
{
  GENET_PRIVATE_DATA *Genet;
  EFI_STATUS Status;
  UINT64 MacAddr;
  
  DEBUG ((EFI_D_INFO, "GenetDriverBindingStart: Entered\n"));

  Genet = AllocateZeroPool (sizeof (GENET_PRIVATE_DATA));
  if (Genet == NULL) {
    DEBUG ((EFI_D_ERROR, "GenetDriverBindingStart: Couldn't allocate private data\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->OpenProtocol (ControllerHandle,
                              &gEfiCallerIdGuid,
                              (VOID **)&Genet->Dev,
                              This->DriverBindingHandle,
                              ControllerHandle,
                              EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GenetDriverBindingStart: Couldn't open protocol: %r\n", Status));
    return Status;
  }

  Status = GenetDmaAlloc (Genet);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GenetDriverBindingStart: Couldn't allocate DMA buffers: %r\n", Status));
    return Status;
  }

  Genet->Signature = GENET_DRIVER_SIGNATURE;
  Genet->RegBase = FixedPcdGet64 (PcdBcmGenetRegistersAddress);
  Genet->Phy.PrivateData = Genet;
  Genet->Phy.Read = GenetPhyRead;
  Genet->Phy.Write = GenetPhyWrite;
  Genet->Phy.Configure = GenetPhyConfigure;
  Genet->PhyMode = GENET_PHY_MODE_RGMII;
  EfiInitializeLock (&Genet->Lock, TPL_CALLBACK);
  Genet->Snp = gGenetSimpleNetwork;
  Genet->Snp.Mode = &Genet->SnpMode;
  Genet->SnpMode.State = EfiSimpleNetworkStopped;
  Genet->SnpMode.HwAddressSize = NET_ETHER_ADDR_LEN;
  Genet->SnpMode.MediaHeaderSize = sizeof (ETHER_HEAD);
  Genet->SnpMode.MaxPacketSize = GENET_MAX_PACKET_SIZE;
  Genet->SnpMode.NvRamSize = 0;
  Genet->SnpMode.NvRamAccessSize = 0;
  Genet->SnpMode.ReceiveFilterMask = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST |
                                     EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST |
                                     EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST |
                                     EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
                                     EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;
  Genet->SnpMode.ReceiveFilterSetting = Genet->SnpMode.ReceiveFilterMask;
  Genet->SnpMode.MaxMCastFilterCount = 0;
  Genet->SnpMode.MCastFilterCount = 0;
  Genet->SnpMode.IfType = NET_IFTYPE_ETHERNET;
  Genet->SnpMode.MacAddressChangeable = TRUE;
  Genet->SnpMode.MultipleTxSupported = FALSE;
  Genet->SnpMode.MediaPresentSupported = TRUE;
  Genet->SnpMode.MediaPresent = FALSE;
  SetMem (&Genet->SnpMode.BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xff);

  MacAddr = PcdGet64 (PcdBcmGenetMacAddress);
  Genet->SnpMode.PermanentAddress = *(EFI_MAC_ADDRESS *)&MacAddr;
  Genet->SnpMode.CurrentAddress = *(EFI_MAC_ADDRESS *)&MacAddr;

  Status = gBS->InstallMultipleProtocolInterfaces (&ControllerHandle,
                                                   &gEfiSimpleNetworkProtocolGuid, &Genet->Snp,
                                                   NULL
                                                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GenetDriverBindingStart: Couldn't install protocol interfaces: %r\n", Status));
    gBS->CloseProtocol (ControllerHandle,
                        &gEfiCallerIdGuid,
                        This->DriverBindingHandle,
                        ControllerHandle);
    FreePool (Genet);
  } else {
    Genet->ControllerHandle = ControllerHandle;
  }

  DEBUG ((EFI_D_INFO, "GenetDriverBindingStart: Returning %r\n", Status));

  return Status;
}

EFI_STATUS
EFIAPI
GenetDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer   OPTIONAL
  )
{
  EFI_SIMPLE_NETWORK_PROTOCOL *SnpProtocol;
  GENET_PRIVATE_DATA *Genet;
  EFI_STATUS Status;

  Status = gBS->HandleProtocol (ControllerHandle,
                                &gEfiSimpleNetworkProtocolGuid,
                                (VOID **)&SnpProtocol
                                );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Genet = GENET_PRIVATE_DATA_FROM_SNP_THIS(This);

  Status = gBS->UninstallMultipleProtocolInterfaces (ControllerHandle,
                                                     &gEfiSimpleNetworkProtocolGuid, &Genet->Snp,
                                                     NULL
                                                     );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GenetDmaFree (Genet);

  FreePages (Genet, EFI_SIZE_TO_PAGES (sizeof (GENET_PRIVATE_DATA)));

  return Status;
}

EFI_STATUS
EFIAPI
GenetEntryPoint (
  IN  EFI_HANDLE          ImageHandle,
  IN  EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_STATUS Status;
  UINT64 MacAddr;
  UINT8 *Bytes;

  MacAddr = PcdGet64 (PcdBcmGenetMacAddress);
  if (MacAddr == 0) {
    DEBUG ((EFI_D_ERROR, "GenetEntryPoint: Hardware not present\n"));
    return EFI_NOT_FOUND;
  }

  Bytes = (UINT8 *)&MacAddr;
  DEBUG ((EFI_D_INFO, "GenetEntryPoint: MAC address %02X:%02X:%02X:%02X:%02X:%02X\n",
    Bytes[0], Bytes[1], Bytes[2], Bytes[3], Bytes[4], Bytes[5]));

  mDevicePath.MacAddrDP.MacAddress = *(EFI_MAC_ADDRESS *)&MacAddr;

  Status = gBS->InstallMultipleProtocolInterfaces (&mDevice,
                                                   &gEfiDevicePathProtocolGuid, &mDevicePath,
                                                   &gEfiCallerIdGuid, NULL,
                                                   NULL
                                                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GenetEntryPoint: InstallMultipleProtocolInterfaces failed: %r\n", Status));
    return Status;
  }

  Status = EfiLibInstallDriverBindingComponentName2 (
    ImageHandle,
    SystemTable,
    &gGenetDriverBinding,
    ImageHandle,
    &gGenetComponentName,
    &gGenetComponentName2
    );
  
  DEBUG ((EFI_D_INFO, "GenetEntryPoint: EfiLibInstallDriverBindingComponentName2 returned %r\n", Status));

  if (EFI_ERROR (Status)) {
    gBS->UninstallMultipleProtocolInterfaces (mDevice,
                                              &gEfiDevicePathProtocolGuid, &mDevicePath,
                                              &gEfiCallerIdGuid, NULL,
                                              NULL);
  }

  return Status;
}
