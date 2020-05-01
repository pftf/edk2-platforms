/** @file
 *
 *  Copyright (c) 2019, Pete Batard <pete@akeo.ie>
 *  Copyright (c) 2017-2018, Andrey Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) 2014, Linaro Limited. All rights reserved.
 *  Copyright (c) 2013-2018, ARM Limited. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/RPiMem.h>
#include <IndustryStandard/Bcm2711.h>
#include <IndustryStandard/Bcm2836.h>

UINT64 mSystemMemoryBase;
extern UINT64 mSystemMemoryEnd;
UINT64 mVideoCoreBase;
UINT64 mVideoCoreSize;
UINT32 mBoardRevision;
//
// gSerialLibCoreClockFreq, which resides in DualSerialLib is set
// to the VPU Core Clock frequency by ArmPlatformPeiBootAction ().
// We use it to report the core frequency during early boot.
//
extern UINT32 gSerialLibCoreClockFreq;

// The total number of descriptors, including the final "end-of-table" descriptor.
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS 11

STATIC BOOLEAN                  VirtualMemoryInfoInitialized = FALSE;
STATIC RPI_MEMORY_REGION_INFO   VirtualMemoryInfo[MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS];

#define VariablesSize (FixedPcdGet32(PcdFlashNvStorageVariableSize) +   \
                       FixedPcdGet32(PcdFlashNvStorageFtwWorkingSize) + \
                       FixedPcdGet32(PcdFlashNvStorageFtwSpareSize) +  \
                       FixedPcdGet32(PcdNvStorageEventLogSize))

#define VariablesBase (FixedPcdGet64(PcdFdBaseAddress) + \
                       FixedPcdGet32(PcdFdSize) - \
                       VariablesSize)

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU
  on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR
                                    describing a Physical-to-Virtual Memory
                                    mapping. This array must be ended by a
                                    zero-filled entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  UINTN                         Index = 0;
  UINTN                         GpuIndex;
  INT64                         OrigMemorySize;
  INT64                         SystemMemorySize;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  // Early output of the info we got from VideoCore can prove valuable.
  DEBUG ((DEBUG_INFO, "Board Rev: 0x%lX\n", mBoardRevision));
  DEBUG ((DEBUG_INFO, "Base RAM : 0x%ll08X (Size 0x%ll08X)\n", mSystemMemoryBase, mSystemMemoryEnd + 1));
  DEBUG ((DEBUG_INFO, "VideoCore: 0x%ll08X (Size 0x%ll08X)\n", mVideoCoreBase, mVideoCoreSize));
  DEBUG ((DEBUG_INFO, "Core Freq: %d MHz\n", gSerialLibCoreClockFreq / 1000000));

  ASSERT (mSystemMemoryBase == 0);
  ASSERT (VirtualMemoryMap != NULL);

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR*)AllocatePages
                       (EFI_SIZE_TO_PAGES (sizeof (ARM_MEMORY_REGION_DESCRIPTOR) *
                       MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (VirtualMemoryTable == NULL) {
    return;
  }


  // Firmware Volume
  VirtualMemoryTable[Index].PhysicalBase    = FixedPcdGet64 (PcdFdBaseAddress);
  VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
  VirtualMemoryTable[Index].Length          = FixedPcdGet32 (PcdFdSize) - VariablesSize;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
  VirtualMemoryInfo[Index].Type             = RPI_MEM_RESERVED_REGION;
  VirtualMemoryInfo[Index++].Name           = L"FD";

  // Variable Volume
  VirtualMemoryTable[Index].PhysicalBase    = VariablesBase;
  VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
  VirtualMemoryTable[Index].Length          = VariablesSize;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
  VirtualMemoryInfo[Index].Type             = RPI_MEM_RUNTIME_REGION;
  VirtualMemoryInfo[Index++].Name           = L"FD Variables";

  //
  // Both the the Pi 4 and Pi 3 implementations expect the DTB to directly follow the FD.
  //
  VirtualMemoryTable[Index].PhysicalBase    = FixedPcdGet32 (PcdFdtBaseAddress);
  VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
  VirtualMemoryTable[Index].Length          = FixedPcdGet32 (PcdFdtSize);;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
  VirtualMemoryInfo[Index].Type             = RPI_MEM_RESERVED_REGION;
  VirtualMemoryInfo[Index++].Name           = L"Flattened Device Tree";
  if (BCM2711_SOC_REGISTERS == 0) {
    //
    // TF-A reserved RAM only exists for the Pi 3 TF-A.
    //
    // This is 2MB that directly follows the FD.
    //
    VirtualMemoryTable[Index].PhysicalBase    = (FixedPcdGet64(PcdFdBaseAddress) + FixedPcdGet32(PcdFdSize));
    VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
    VirtualMemoryTable[Index].Length          = FixedPcdGet64 (PcdSystemMemoryBase) - VirtualMemoryTable[Index].PhysicalBase;
    VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
    VirtualMemoryInfo[Index].Type             = RPI_MEM_RESERVED_REGION;
    VirtualMemoryInfo[Index++].Name           = L"TF-A RAM";
  }

  // Base System RAM
  VirtualMemoryTable[Index].PhysicalBase    = FixedPcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
  VirtualMemoryTable[Index].Length          = mSystemMemoryEnd + 1 - FixedPcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
  VirtualMemoryInfo[Index].Type             = RPI_MEM_BASIC_REGION;
  VirtualMemoryInfo[Index++].Name           = L"Base System RAM";

  // GPU Reserved
  GpuIndex = Index;
  VirtualMemoryTable[Index].PhysicalBase    = mVideoCoreBase;
  VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
  VirtualMemoryTable[Index].Length          = mVideoCoreSize;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;
  VirtualMemoryInfo[Index].Type             = RPI_MEM_UNMAPPED_REGION;
  VirtualMemoryInfo[Index++].Name           = L"GPU Reserved";

  // Compute the total RAM size available on this platform
  SystemMemorySize = SIZE_256MB;
  SystemMemorySize <<= (mBoardRevision >> 20) & 0x07;

  //
  // Ensure that what we declare as System Memory doesn't overlap with the
  // Bcm2836 SoC registers. This can be achieved through a MIN () with the
  // base address since SystemMemoryBase is 0 (we assert if it isn't).
  //
  SystemMemorySize = MIN(SystemMemorySize, BCM2836_SOC_REGISTERS);

  // Extended SoC registers (PCIe, genet, ...)
  if (BCM2711_SOC_REGISTERS > 0) {
    // Same overlap protection as above for the Bcm2711 SoC registers
    SystemMemorySize                        = MIN(SystemMemorySize, BCM2711_SOC_REGISTERS);
    VirtualMemoryTable[Index].PhysicalBase  = BCM2711_SOC_REGISTERS;
    VirtualMemoryTable[Index].VirtualBase   = VirtualMemoryTable[Index].PhysicalBase;
    VirtualMemoryTable[Index].Length        = BCM2711_SOC_REGISTER_LENGTH;
    VirtualMemoryTable[Index].Attributes    = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;
    VirtualMemoryInfo[Index].Type           = RPI_MEM_UNMAPPED_REGION;
    VirtualMemoryInfo[Index++].Name         = L"SoC Reserved (27xx)";
  }

  // Base SoC registers
  VirtualMemoryTable[Index].PhysicalBase    = BCM2836_SOC_REGISTERS;
  // On the Pi 3 the SoC registers may overlap VideoCore => fix this
  if (VirtualMemoryTable[GpuIndex].PhysicalBase + VirtualMemoryTable[GpuIndex].Length > VirtualMemoryTable[Index].PhysicalBase) {
    VirtualMemoryTable[GpuIndex].Length = VirtualMemoryTable[Index].PhysicalBase - VirtualMemoryTable[GpuIndex].PhysicalBase;
  }
  VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
  VirtualMemoryTable[Index].Length          = BCM2836_SOC_REGISTER_LENGTH;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;
  VirtualMemoryInfo[Index].Type             = RPI_MEM_UNMAPPED_REGION;
  VirtualMemoryInfo[Index++].Name           = L"SoC Reserved (283x)";

  //
  // By default we limit the memory to 3 GB to work around the DMA bugs in the SoC,
  // for OSes that don't support _DMA range descriptors. On 4GB boards, it's runtime
  // setting to boot with 4 GB, and the additional 1 GB is added by ConfigDxe.
  //
  OrigMemorySize = SystemMemorySize;
  SystemMemorySize = MIN(SystemMemorySize, 3UL * SIZE_1GB);

  // If we have RAM above the 1 GB mark, declare it
  if (SystemMemorySize - SIZE_1GB > 0) {
    VirtualMemoryTable[Index].PhysicalBase  = FixedPcdGet64 (PcdExtendedMemoryBase);
    VirtualMemoryTable[Index].VirtualBase   = VirtualMemoryTable[Index].PhysicalBase;
    VirtualMemoryTable[Index].Length        = SystemMemorySize - SIZE_1GB;
    VirtualMemoryTable[Index].Attributes    = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
    VirtualMemoryInfo[Index].Type           = RPI_MEM_BASIC_REGION;
    VirtualMemoryInfo[Index++].Name         = L"Extended System RAM below 3 GB";
  }

  //
  // If we have RAM above 3 GB mark, declare it so it's mapped, but
  // don't add it to the memory map. This is done later by ConfigDxe if necessary.
  //
  if (OrigMemorySize > (3UL * SIZE_1GB)) {
    VirtualMemoryTable[Index].PhysicalBase  = 3UL * SIZE_1GB;
    VirtualMemoryTable[Index].VirtualBase   = VirtualMemoryTable[Index].PhysicalBase;
    VirtualMemoryTable[Index].Length        = OrigMemorySize - VirtualMemoryTable[Index].PhysicalBase;
    VirtualMemoryTable[Index].Attributes    = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
    VirtualMemoryInfo[Index].Type           = RPI_MEM_UNMAPPED_REGION;
    VirtualMemoryInfo[Index++].Name         = L"Extended System RAM above 3 GB";
  }

  // End of Table
  VirtualMemoryTable[Index].PhysicalBase    = 0;
  VirtualMemoryTable[Index].VirtualBase     = 0;
  VirtualMemoryTable[Index].Length          = 0;
  VirtualMemoryTable[Index++].Attributes    = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT(Index <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
  VirtualMemoryInfoInitialized = TRUE;
}

/**
  Return additional memory info not populated by the above call.

  This call should follow the one to ArmPlatformGetVirtualMemoryMap ().

**/
VOID
RpiPlatformGetVirtualMemoryInfo (
  IN RPI_MEMORY_REGION_INFO** MemoryInfo
  )
{
  ASSERT (VirtualMemoryInfo != NULL);

  if (!VirtualMemoryInfoInitialized) {
    DEBUG ((DEBUG_ERROR,
      "ArmPlatformGetVirtualMemoryMap must be called before RpiPlatformGetVirtualMemoryInfo.\n"));
    return;
  }

  *MemoryInfo = VirtualMemoryInfo;
}
