/** @file
  <<BriefDescription>>
  <<DetailedDescription>>
  <<Copyright>>
  <<License>>
**/

#include "Genet.h"
#include "ComponentName.h"

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
EFI_UNICODE_STRING_TABLE mGenetNameTable[] = {
    {
        "eng;en",
        L"SNP Broadcom GENET Driver"
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
        mGenetNameTable,
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
    return EFI_UNSUPPORTED;
}