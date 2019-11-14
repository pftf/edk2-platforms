/** @file
  PCI Host Bridge Library instance for Bcm2711 ARM SOC

  Copyright (c) 2017, Linaro Ltd. All rights reserved.
  Copyright (c) 2019, Jeremy Linton

  SPDX-License-Identifier: BSD-2-Clause-Patent

  This module initializes the Pci as close to a standard
  pci root complex as possible.
  See: https://github.com/raspberrypi/linux/blob/rpi-5.3.y/drivers/pci/controller/pcie-brcmstb.c
  and
  https://github.com/raspberrypi/linux/blob/rpi-5.3.y/arch/arm/boot/dts/bcm2838.dtsi
**/

#include <PiDxe.h>
#include <IndustryStandard/Pci22.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/UefiBootServicesTableLib.h>
//#include <Platform/Pcie.h>
//#include <Platform/VarStore.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <IndustryStandard/Bcm2711.h>

static UINT32 bogus_catch;

STATIC
UINT32
RdRegister (
  UINT32                Offset,
  UINT32                Mask
  )
{
  EFI_PHYSICAL_ADDRESS  Base = PCIE_REG_BASE;

  ArmDataMemoryBarrier ();
  if (!Mask) //hmmm maybe C++'s default parms are useful?
      Mask = (UINT32)-1;

  return MmioRead32 (Base + Offset) & Mask;
}


STATIC
UINT32
RdRegister16 (
  UINT32                Offset,
  UINT32                Mask
  )
{
  EFI_PHYSICAL_ADDRESS  Base = PCIE_REG_BASE;

  ArmDataMemoryBarrier ();
  if (!Mask) //hmmm maybe C++'s default parms are useful?
      Mask = (UINT32)-1;

  return MmioRead16 (Base + Offset) & Mask;
}


STATIC
VOID
RMWRegister (
  UINT32                Offset,
  UINT32                Mask,
  UINT32                In
  )
{
  EFI_PHYSICAL_ADDRESS  Addr = PCIE_REG_BASE;
  UINT32 Data = 0;
  UINT32 Shift;

  Addr += Offset;
  //DEBUG ((DEBUG_ERROR, "RootBridge pci read %x from %x\n", Data, Addr));
  Shift = 1;
  if (In) {
    while (!(Mask & Shift))
      Shift <<= 1;
    Data = (MmioRead32 (Addr) & ~Mask) | (In * Shift);
  } else {
    Data = MmioRead32 (Addr) & ~Mask;
  }

  DEBUG ((DEBUG_ERROR, "RootBridge pci write %x to %x\n", Data, Addr));
  MmioWrite32 (Addr, Data);

  ArmDataMemoryBarrier ();
  bogus_catch = RdRegister(Offset, 0);
}


STATIC
VOID
WdRegister (
  UINT32                Offset,
  UINT32                In
  )
{
  EFI_PHYSICAL_ADDRESS  Base = PCIE_REG_BASE;

  MmioWrite32 (Base + Offset, In);
  DEBUG ((DEBUG_ERROR, "RootBridge pci write %x to %x\n", In, Base+Offset));

  ArmDataMemoryBarrier ();
  bogus_catch = RdRegister(Offset, 0);
}

/*
STATIC
VOID
WdRegister16 (
  UINT32                Offset,
  UINT32                In
  )
{
  EFI_PHYSICAL_ADDRESS  Base = PCIE_REG_BASE;

  MmioWrite16 (Base + Offset, In);
  ArmDataMemoryBarrier ();    
}
*/




EFI_STATUS
EFIAPI
Bcm2711PciHostBridgeLibConstructor (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  UINT32 Data;
//  EFI_PHYSICAL_ADDRESS Reg;
  int timeout = 100;
  EFI_PHYSICAL_ADDRESS  TopOfPciMap; 

  
  DEBUG ((DEBUG_ERROR, "RootBridge constructor\n"));
  
  // Reset controller
  RMWRegister (PCIE_RGR1_SW_INIT_1, PCIE_RGR1_SW_INIT_1_INIT_MASK, 1);
  // PERST
  RMWRegister (PCIE_RGR1_SW_INIT_1, PCIE_RGR1_SW_INIT_1_PERST_MASK, 1);

  gBS->Stall (1000);
  // take the bridge out of reset
  RMWRegister (PCIE_RGR1_SW_INIT_1, PCIE_RGR1_SW_INIT_1_INIT_MASK, 0);

  
  RMWRegister(PCIE_MISC_HARD_PCIE_HARD_DEBUG, PCIE_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ_MASK, 0);
  RdRegister(PCIE_MISC_HARD_PCIE_HARD_DEBUG, 0);
  // Wait for SerDes to be stable
  gBS->Stall (1000);
    
  // read revision
  Data = RdRegister (PCIE_MISC_REVISION, PCIE_MISC_REVISION_MAJMIN_MASK); //0xffff
  DEBUG ((DEBUG_ERROR, "RootBridge: Revision %x\n", Data));

  RMWRegister (PCIE_MISC_MISC_CTRL, PCIE_MISC_MISC_CTRL_SCB_ACCESS_EN_MASK, 1);
  RMWRegister (PCIE_MISC_MISC_CTRL, PCIE_MISC_MISC_CTRL_CFG_READ_UR_MODE_MASK, 1);
  RMWRegister (PCIE_MISC_MISC_CTRL, PCIE_MISC_MISC_CTRL_MAX_BURST_SIZE_MASK, BURST_SIZE_256);

  // "RC_BAR2" is the inbound TLP window, apparently
  // having non RAM regions in the window is ok (and
  // encouraged? for PtP?)
  // so lets just map the entire address space..

  // for regions > 64K then the pci->mem window size = log2(size)-15
  // which is dumped into the low bits of the offset and written to the "LO" register
  // with the high bits of the offset written into the "HI" part.
  // the linux driver makes the point that the offset must be aligned to its size
  // aka a 1G region must start on a 1G boundary

  DEBUG ((DEBUG_ERROR, "RootBridge: Program bottom 2G of ram\n"));
  
  
  // lets assume a start addr of 0, size 2G
  WdRegister (PCIE_MISC_RC_BAR2_CONFIG_LO, 0x10); //size =2 G
  WdRegister (PCIE_MISC_RC_BAR2_CONFIG_HI, 0);    //start at addr0
  RMWRegister(PCIE_MISC_MISC_CTRL, PCIE_MISC_MISC_CTRL_SCB0_SIZE_MASK, 0x10); //1GB=0xf=log2(size)-15), 4G=0x11
  
  // RC_BAR1 pcie->gisb disable
  WdRegister (PCIE_MISC_RC_BAR1_CONFIG_LO, 0);
  // RC_BAR3 pcie->scb disable
  WdRegister (PCIE_MISC_RC_BAR3_CONFIG_LO, 0);

  TopOfPciMap = PCIE_TOP_OF_MEM_WIN;

  DEBUG ((DEBUG_ERROR, "RootBridge: MMIO PCIe addr %llx\n", TopOfPciMap));
  // See brcm_pcie_set_outbound_win() in the raspberrypi tree
  WdRegister (PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LO, TopOfPciMap);
  WdRegister (PCIE_MISC_CPU_2_PCIE_MEM_WIN0_HI, TopOfPciMap >> 32); //4GB? and bounce or just map the whole thing?
  // everyone else is doing this following PERST but why not do it at the same time as the other addr windows?
  // Set up the CPU addresses.
  // The low register holds the bottom part of the start and end addresses as
  // two 12-bit values expressed in megabytes:
  // | low_end |  0  | low_start | 0 |
  // | 31    20|   16|          4|  0|
  // Note that the end address gets truncated at the megabyte boundary, so
  // 0x1000000 bytes will become 0xf megabytes.
  // There are two high registers for holding the top 32-bits of the start and
  // end addresses, respectively.
  // Of course, this is all speculation in the absence of an official
  // datasheet.
  // Further these addrss ranges are borrowed and seem less than ideal
  // the mapping should be 1:1 if possible
  EFI_PHYSICAL_ADDRESS    cpu_addr_start = PCIE_CPU_MMIO_WINDOW;
  EFI_PHYSICAL_ADDRESS    cpu_addr_end = cpu_addr_start + PCIE_BRIDGE_MMIO_WINDOW;
  EFI_PHYSICAL_ADDRESS    cpu_addr_low =  ((cpu_addr_start >> 16) & 0xffff) | (((cpu_addr_end >> 20) - 1) << 20);

  DEBUG ((DEBUG_ERROR, "RootBridge: MMIO CPU addr %llx\n", cpu_addr_start));
  
  WdRegister( PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT, cpu_addr_low);
  WdRegister( PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI, cpu_addr_start >> 32);
  WdRegister( PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI, cpu_addr_end >> 32);

  //EFI_PHYSICAL_ADDRESS    cpu_addr_start = PCIE_CPU_MMIO_WINDOW>>20;
  //EFI_PHYSICAL_ADDRESS    cpu_addr_end = (cpu_addr_start + PCIE_BRIDGE_MMIO_WINDOW-1)>>20;
  
  //DEBUG ((DEBUG_ERROR, "RootBridge: MMIO CPU addr %llx\n", cpu_addr_start));
  
  //RMWRegister (PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT, PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_BASE_MASK, cpu_addr_start );
  //RMWRegister (PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT, PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_LIMIT_MASK, cpu_addr_end );

  //RMWRegister (PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI, PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI_BASE_MASK, cpu_addr_start>>PCIE_MISC_CPU_2_PCIE_MEM_WIN0_MASK_BITS );
  //RMWRegister (PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI, PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI_LIMIT_MASK, cpu_addr_end>>PCIE_MISC_CPU_2_PCIE_MEM_WIN0_MASK_BITS );
  

  // Consider MSI setup here, not that it matters much its likely the legacy intX
  // is as fast or faster...
  DEBUG ((DEBUG_ERROR, "RootBridge: clear ints\n"));
 
  // Clear and mask interrupts.
  WdRegister(PCIE_INTR2_CPU_MASK_CLR, 0xffffffff);
  WdRegister(PCIE_INTR2_CPU_MASK_SET, 0xffffffff);

  // set link cap & link ctl
  //RMWRegister (BRCM_PCIE_CAP_REGS+PCI_LNKCAP, LNKCAP, pen);
  //RMWRegister (BRCM_PCIE_CTL_REGS+PCI_LNKCAP, LNKCAP, pen);

  // de-assert PERST
  RMWRegister (PCIE_RGR1_SW_INIT_1, PCIE_RGR1_SW_INIT_1_PERST_MASK, 0);
  DEBUG ((DEBUG_ERROR, "RootBridge: Reset done\n"));

  // wait for linkup
  do {
      Data = RdRegister (PCIE_MISC_PCIE_STATUS, 0);
      gBS->Stall (1000); 
      timeout --;
  } while (((Data & 0x30) != 0x030) && (timeout));
  DEBUG ((DEBUG_ERROR, "PCIe link ready (status=%x) timeout=%d\n", Data, timeout));

  if ((Data & 0x30) != 0x30) {
    DEBUG ((DEBUG_ERROR, "PCIe link not ready (status=%x)\n", Data));
    return EFI_DEVICE_ERROR;
  }

  if ((Data & 0x80) != 0x80) {
    DEBUG ((DEBUG_ERROR, "PCIe link not in RC mode (status=%x)\n", Data));
    return EFI_UNSUPPORTED;
  }

  // Change class code?
  RMWRegister(BRCM_PCIE_CLASS, PCIE_RC_CFG_PRIV1_ID_VAL3_CLASS_CODE_MASK, 0x60400);
  //WdRegister16(BRCM_PCIE_CLASS, 0x60400);

  Data = RdRegister16(BRCM_PCIE_CAP_REGS + PCI_EXP_LNKSTA, 0);
  DEBUG((DEBUG_ERROR, "link up, %d Gbps x%u\n", Data & PCI_EXP_LNKSTA_CLS, (Data & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT));

  /* PCIe->SCB endian mode for BAR */
  /* field ENDIAN_MODE_BAR2 = DATA_ENDIAN */
  RMWRegister( PCIE_RC_CFG_VENDOR_VENDOR_SPECIFIC_REG1, PCIE_RC_CFG_VENDOR_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR2_MASK, 0);
  
  return EFI_SUCCESS;
}
