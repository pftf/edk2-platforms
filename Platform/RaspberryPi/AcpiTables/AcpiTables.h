/** @file
 *
 *  RPi defines for constructing ACPI tables
 *
 *  Copyright (c) 2020, Pete Batard <pete@akeo.ie>
 *  Copyright (c) 2019, ARM Ltd. All rights reserved.
 *  Copyright (c) 2018, Andrei Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __ACPITABLES_H__
#define __ACPITABLES_H__

#include <IndustryStandard/Acpi.h>

// The ASL compiler can't perform arithmetic on MEMORY32SETBASE ()
// parameters so you can't pass a constant like BASE + OFFSET (the
// compiler just silently sets it to zero). So we need a macro that
// can perform arithmetic base address update with an offset.
#define MEMORY32SETBASE(BufName, MemName, VarName, Offset)       \
    CreateDwordField (^BufName, ^MemName._BAS, VarName)          \
    Add (BCM2836_SOC_REGISTERS, Offset, VarName)

#if (RPI_MODEL == 3)
#define EFI_ACPI_OEM_ID                       {'B','C','2','8','3','6'}
#else
#define EFI_ACPI_OEM_ID                       {'M','C','R','S','F','T'}
#endif
#define EFI_ACPI_OEM_TABLE_ID                 SIGNATURE_64 ('R','P','I','_','E','D','K','2')
#define EFI_ACPI_OEM_REVISION                 0x00000100
#define EFI_ACPI_CREATOR_ID                   SIGNATURE_32 ('E','D','K','2')
#define EFI_ACPI_CREATOR_REVISION             0x00000100

#define EFI_ACPI_VENDOR_ID                    SIGNATURE_32 ('M','S','F','T')

// A macro to initialise the common header part of EFI ACPI tables as defined by
// EFI_ACPI_DESCRIPTION_HEADER structure.
#define ACPI_HEADER(Signature, Type, Revision) {                  \
    Signature,                      /* UINT32  Signature */       \
    sizeof (Type),                  /* UINT32  Length */          \
    Revision,                       /* UINT8   Revision */        \
    0,                              /* UINT8   Checksum */        \
    EFI_ACPI_OEM_ID,                /* UINT8   OemId[6] */        \
    EFI_ACPI_OEM_TABLE_ID,          /* UINT64  OemTableId */      \
    EFI_ACPI_OEM_REVISION,          /* UINT32  OemRevision */     \
    EFI_ACPI_CREATOR_ID,            /* UINT32  CreatorId */       \
    EFI_ACPI_CREATOR_REVISION       /* UINT32  CreatorRevision */ \
  }

#define EFI_ACPI_CSRT_REVISION                0x00000005
#define EFI_ACPI_CSRT_DEVICE_ID_DMA           0x00000009 // Fixed id
#define EFI_ACPI_CSRT_RESOURCE_ID_IN_DMA_GRP  0x0 // Count up from 0

#define RPI_DMA_CHANNEL_COUNT                 10 // All 10 DMA channels are listed, including the reserved ones
#define RPI_DMA_USED_CHANNEL_COUNT            5  // Use 5 DMA channels

#if (RPI_MODEL == 3)
#define RPI_SYSTEM_TIMER_BASE_ADDRESS         0x4000001C
#elif (RPI_MODEL == 4)
#define RPI_SYSTEM_TIMER_BASE_ADDRESS         0xFF80001C
#endif

#define EFI_ACPI_5_1_CSRT_REVISION            0x00000000

typedef enum
{
  EFI_ACPI_CSRT_RESOURCE_TYPE_RESERVED,           // 0
  EFI_ACPI_CSRT_RESOURCE_TYPE_INTERRUPT,          // 1
  EFI_ACPI_CSRT_RESOURCE_TYPE_TIMER,              // 2
  EFI_ACPI_CSRT_RESOURCE_TYPE_DMA,                // 3
  EFI_ACPI_CSRT_RESOURCE_TYPE_CACHE,              // 4
}
CSRT_RESOURCE_TYPE;

typedef enum
{
  EFI_ACPI_CSRT_RESOURCE_SUBTYPE_DMA_CHANNEL,     // 0
  EFI_ACPI_CSRT_RESOURCE_SUBTYPE_DMA_CONTROLLER   // 1
}
CSRT_DMA_SUBTYPE;

//------------------------------------------------------------------------
// CSRT Resource Group header 24 bytes long
//------------------------------------------------------------------------
typedef struct
{
  UINT32 Length;                  // Length
  UINT32 VendorID;                // 4 bytes
  UINT32 SubVendorId;             // 4 bytes
  UINT16 DeviceId;                // 2 bytes
  UINT16 SubdeviceId;             // 2 bytes
  UINT16 Revision;                // 2 bytes
  UINT16 Reserved;                // 2 bytes
  UINT32 SharedInfoLength;        // 4 bytes
} EFI_ACPI_5_1_CSRT_RESOURCE_GROUP_HEADER;

//------------------------------------------------------------------------
// CSRT Resource Descriptor 12 bytes total
//------------------------------------------------------------------------
typedef struct
{
  UINT32 Length;                  // 4 bytes
  UINT16 ResourceType;            // 2 bytes
  UINT16 ResourceSubType;         // 2 bytes
  UINT32 UID;                     // 4 bytes
} EFI_ACPI_5_1_CSRT_RESOURCE_DESCRIPTOR_HEADER;

//------------------------------------------------------------------------
// Interrupts. These are specific to each platform
//------------------------------------------------------------------------
#if (RPI_MODEL == 3)
#define BCM2836_V3D_BUS_INTERRUPT               0x2A
#define BCM2836_DMA_INTERRUPT                   0x3B
#define BCM2836_SPI1_INTERRUPT                  0x3D
#define BCM2836_SPI2_INTERRUPT                  0x3D
#define BCM2836_HVS_INTERRUPT                   0x41
#define BCM2836_HDMI0_INTERRUPT                 0x48
#define BCM2836_HDMI1_INTERRUPT                 0x49
#define BCM2836_PV2_INTERRUPT                   0x4A
#define BCM2836_PV0_INTERRUPT                   0x4D
#define BCM2836_PV1_INTERRUPT                   0x4E
#define BCM2836_MBOX_INTERRUPT                  0x61
#define BCM2836_VCHIQ_INTERRUPT                 0x62
#define BCM2386_GPIO_INTERRUPT0                 0x51
#define BCM2386_GPIO_INTERRUPT1                 0x53
#define BCM2836_I2C1_INTERRUPT                  0x55
#define BCM2836_I2C2_INTERRUPT                  0x55
#define BCM2836_SPI0_INTERRUPT                  0x56
#define BCM2836_USB_INTERRUPT                   0x29
#define BCM2836_SDHOST_INTERRUPT                0x58
#define BCM2836_MMCHS1_INTERRUPT                0x5E
#define BCM2836_MINI_UART_INTERRUPT             0x3D
#define BCM2836_PL011_UART_INTERRUPT            0x59
#define CORTEX_L1D_SIZE                         SIZE_16KB
#define CORTEX_L1D_SETS                         64
#define CORTEX_L1D_ASSC                         4
#define CORTEX_L1I_SIZE                         SIZE_16KB
#define CORTEX_L1I_SETS                         128
#define CORTEX_L1I_ASSC                         2
#define CORTEX_L2_SIZE                          SIZE_512KB
#define CORTEX_L2_SETS                          512
#define CORTEX_L2_ASSC                          16
#elif (RPI_MODEL == 4)
#define BCM2836_V3D_BUS_INTERRUPT               0x2A
#define BCM2836_DMA_INTERRUPT                   0x3B
#define BCM2836_SPI1_INTERRUPT                  0x3D
#define BCM2836_SPI2_INTERRUPT                  0x3D
#define BCM2836_HVS_INTERRUPT                   0x41
#define BCM2836_HDMI0_INTERRUPT                 0x48
#define BCM2836_HDMI1_INTERRUPT                 0x49
#define BCM2836_PV2_INTERRUPT                   0x4A
#define BCM2836_PV0_INTERRUPT                   0x4D
#define BCM2836_PV1_INTERRUPT                   0x4E
#define BCM2836_MBOX_INTERRUPT                  0x61
#define BCM2836_VCHIQ_INTERRUPT                 0x62
#define BCM2386_GPIO_INTERRUPT0                 0x51
#define BCM2386_GPIO_INTERRUPT1                 0x53
#define BCM2836_I2C1_INTERRUPT                  0x55
#define BCM2836_I2C2_INTERRUPT                  0x55
#define BCM2836_SPI0_INTERRUPT                  0x56
#define BCM2836_USB_INTERRUPT                   0x69
#define BCM2836_SDHOST_INTERRUPT                0x98
#define BCM2836_MMCHS1_INTERRUPT                0x9E
#define BCM2836_MINI_UART_INTERRUPT             0x7D
#define BCM2836_PL011_UART_INTERRUPT            0x99
#define GENET_INTERRUPT0                        0xBD
#define GENET_INTERRUPT1                        0xBE
#define CORTEX_L1D_SIZE                         SIZE_32KB
#define CORTEX_L1D_SETS                         256
#define CORTEX_L1D_ASSC                         2
#define CORTEX_L1I_SIZE                         (3*SIZE_16KB)
#define CORTEX_L1I_SETS                         256
#define CORTEX_L1I_ASSC                         3
#define CORTEX_L2_SIZE                          SIZE_1MB
#define CORTEX_L2_SETS                          1024
#define CORTEX_L2_ASSC                          16
#endif

#endif // __ACPITABLES_H__
