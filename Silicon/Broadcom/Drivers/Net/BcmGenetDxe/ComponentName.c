/** @file
  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause
**/

#include "GenetUtil.h"

EFI_STATUS
EFIAPI
GenetComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **DriverName
  );

EFI_STATUS
EFIAPI
GenetComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_HANDLE                    ChildHandle,  OPTIONAL
  IN  CHAR8                         *Language,
  OUT CHAR16                        **ControllerName
  );

GLOBAL_REMOVE_IF_UNREFERENCED
EFI_COMPONENT_NAME_PROTOCOL gGenetComponentName = {
  (EFI_COMPONENT_NAME_GET_DRIVER_NAME) GenetComponentNameGetDriverName,
  (EFI_COMPONENT_NAME_GET_CONTROLLER_NAME) GenetComponentNameGetControllerName,
  "eng"
};

GLOBAL_REMOVE_IF_UNREFERENCED
EFI_COMPONENT_NAME2_PROTOCOL gGenetComponentName2 = {
  GenetComponentNameGetDriverName,
  GenetComponentNameGetControllerName,
  "en"
};

STATIC
EFI_UNICODE_STRING_TABLE mGenetDriverNameTable[] = {
    {
        "eng;en",
        L"Broadcom GENET Ethernet Driver"
    },
    {
        NULL,
        NULL
    }
};

STATIC
EFI_UNICODE_STRING_TABLE mGenetDeviceNameTable[] = {
    {
        "eng;en",
        L"Broadcom GENET Ethernet"
    },
    {
        NULL,
        NULL
    }
};

EFI_STATUS
EFIAPI
GenetComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **DriverName
  )
{
    return LookupUnicodeString2 (
        Language,
        This->SupportedLanguages,
        mGenetDriverNameTable,
        DriverName,
        (BOOLEAN)(This == &gGenetComponentName2)
        );
}

EFI_STATUS
EFIAPI
GenetComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_HANDLE                    ChildHandle,  OPTIONAL
  IN  CHAR8                         *Language,
  OUT CHAR16                        **ControllerName
  )
{
    if (ChildHandle != NULL) {
        return EFI_UNSUPPORTED;
    }

    return LookupUnicodeString2 (
        Language,
        This->SupportedLanguages,
        mGenetDeviceNameTable,
        ControllerName,
        (BOOLEAN)(This == &gGenetComponentName2)
        );
}