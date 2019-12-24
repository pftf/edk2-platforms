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

#define ALIGN_DOWN_VALUE(Value, Alignment) ((Value) & ~((Alignment) - 1))

UINT64 mSystemMemoryBase;
extern UINT64 mSystemMemoryEnd;
UINT64 mVideoCoreBase;
UINT64 mVideoCoreSize;
UINT32 mBoardRevision;


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

#define ATFBase (FixedPcdGet64(PcdFdBaseAddress) + FixedPcdGet32(PcdFdSize))

STATIC UINT32
GetModelFamily (VOID)
{
  switch ((mBoardRevision >> 4) & 0xFF) {
    // www.raspberrypi.org/documentation/hardware/raspberrypi/revision-codes/README.md
  case 0x00:          // Raspberry Pi Model A
  case 0x01:          // Raspberry Pi Model B
  case 0x02:          // Raspberry Pi Model A+
  case 0x03:          // Raspberry Pi Model B+
  case 0x06:          // Raspberry Pi Compute Module 1
  case 0x09:          // Raspberry Pi Zero
  case 0x0C:          // Raspberry Pi Zero W
    return 1;
  case 0x04:          // Raspberry Pi 2 Model B
    return 2;
  case 0x08:          // Raspberry Pi 3 Model B
  case 0x0A:          // Raspberry Pi Compute Module 3
  case 0x0D:          // Raspberry Pi 3 Model B+
  case 0x0E:          // Raspberry Pi 3 Model A+
  case 0x10:          // Raspberry Pi Compute Module 3+
    return 3;
      break;
  case 0x11:          // Raspberry Pi 4 Model B
    return 4;

  }

  // Unknown.
  return 0;
}

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
  INT64                         SystemMemorySize;
  UINTN                         BaseMemorySize;
  UINTN                         BaseMemoryStart;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  // Early output of the info we got from VideoCore can prove valuable.
  DEBUG ((DEBUG_INFO, "Board Rev: 0x%lX\n", mBoardRevision));
  DEBUG ((DEBUG_INFO, "Base RAM : 0x%ll08X (Size 0x%ll08X)\n", mSystemMemoryBase, mSystemMemoryEnd + 1));
  DEBUG ((DEBUG_INFO, "VideoCore: 0x%ll08X (Size 0x%ll08X)\n", mVideoCoreBase, mVideoCoreSize));

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

  // TF-A reserved RAM
  VirtualMemoryTable[Index].PhysicalBase    = ATFBase;
  VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
  VirtualMemoryTable[Index].Length          = FixedPcdGet64 (PcdSystemMemoryBase) - ATFBase;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
  VirtualMemoryInfo[Index].Type             = RPI_MEM_RESERVED_REGION;
  VirtualMemoryInfo[Index++].Name           = L"TF-A RAM";

  BaseMemoryStart = FixedPcdGet64 (PcdSystemMemoryBase);
  if (GetModelFamily () == 4) {
    VirtualMemoryTable[Index].PhysicalBase    = BaseMemoryStart;
    VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
    VirtualMemoryTable[Index].Length          = 0x100000;
    BaseMemoryStart += VirtualMemoryTable[Index].Length;
    VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;
    VirtualMemoryInfo[Index].Type             = RPI_MEM_UNMAPPED_REGION;
    VirtualMemoryInfo[Index++].Name           = L"Fake ECAM";
  }

  BaseMemorySize = mSystemMemoryEnd + 1 - BaseMemoryStart;
  
  // Base System RAM
  VirtualMemoryTable[Index].PhysicalBase    = BaseMemoryStart;
  VirtualMemoryTable[Index].VirtualBase     = VirtualMemoryTable[Index].PhysicalBase;
  VirtualMemoryTable[Index].Length          = BaseMemorySize;
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

  if (FeaturePcdGet (PcdAcpiBasicMode)) {
    //
    // Limit the memory to 3 GB to work around the DMA bugs in the SoC without
    // having to rely on IORT or _DMA descriptions.
    //
    SystemMemorySize = MIN(SystemMemorySize, 3U * SIZE_1GB);
  }

  // If we have RAM above the 1 GB mark, declare it
  if (SystemMemorySize - SIZE_1GB > 0) {
    VirtualMemoryTable[Index].PhysicalBase  = FixedPcdGet64 (PcdExtendedMemoryBase);
    VirtualMemoryTable[Index].VirtualBase   = VirtualMemoryTable[Index].PhysicalBase;
    VirtualMemoryTable[Index].Length        = SystemMemorySize - SIZE_1GB;
    VirtualMemoryTable[Index].Attributes    = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
    VirtualMemoryInfo[Index].Type           = RPI_MEM_BASIC_REGION;
    VirtualMemoryInfo[Index++].Name         = L"Extended System RAM";
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
