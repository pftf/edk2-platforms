/** @file
  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause
**/

#ifndef GENET_COMPONENTNAME_H__
#define GENET_COMPONENTNAME_H__

extern EFI_COMPONENT_NAME_PROTOCOL            gGenetComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL           gGenetComponentName2;

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

#endif // GENET_COMPONENTNAME_H__