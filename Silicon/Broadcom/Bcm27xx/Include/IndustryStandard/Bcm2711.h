/** @file
 *
 *  Copyright (c) 2019, Jeremy Linton
 *  Copyright (c) 2019, Pete Batard <pete@akeo.ie>.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __BCM2711_H__
#define __BCM2711_H__

#define BCM2711_SOC_REGISTERS               (FixedPcdGet64 (PcdBcm27xxRegistersAddress))
#define BCM2711_SOC_REGISTER_LENGTH         0x02000000

// Generic BCM pci addrs
#define PCIE_TOP_OF_MEM_WIN    0xf8000000UL
#define PCIE_CPU_MMIO_WINDOW  0x600000000UL
#define PCIE_BRIDGE_MMIO_WINDOW 0x4000000UL

// pci root bridge control registers location
#define PCIE_REG_BASE  0xfd500000 //base post translation? not working at the moment
#define PCIE_REG_LIMIT 0x9310

// pci root bridge control registers
#define BRCM_PCIE_CAP_REGS               0x00ac //offset to ecam like range for root port
#define PCIE_RC_CFG_VENDOR_VENDOR_SPECIFIC_REG1     0x0188
#define BRCM_PCIE_CLASS                  0x043c //in rpi code?
#define PCIE_MISC_MISC_CTRL              0x4008
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LO 0x400c
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_HI 0x4010
#define PCIE_MISC_RC_BAR1_CONFIG_LO      0x402c
#define PCIE_MISC_RC_BAR2_CONFIG_LO      0x4034
#define PCIE_MISC_RC_BAR2_CONFIG_HI      0x4038
#define PCIE_MISC_RC_BAR3_CONFIG_LO      0x403c
#define PCIE_MISC_PCIE_STATUS            0x4068
#define PCIE_MISC_REVISION               0x406c
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT    0x4070
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI       0x4080
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI      0x4084
#define PCIE_MISC_HARD_PCIE_HARD_DEBUG  0x4204

#define PCIE_INTR2_CPU_STATUS       0x4300
#define PCIE_INTR2_CPU_SET          0x4304
#define PCIE_INTR2_CPU_CLR          0x4308
#define PCIE_INTR2_CPU_MASK_STATUS  0x430c
#define PCIE_INTR2_CPU_MASK_SET     0x4310
#define PCIE_INTR2_CPU_MASK_CLR     0x4314

#define PCIE_RGR1_SW_INIT_1         0x9210
#define PCIE_EXT_CFG_INDEX          0x9000
#define PCIE_EXT_CFG_DATA           0x8000 //a small window pointing at the ECAM of the device selected by CFG_INDEX

#define PCIE_RC_CFG_VENDOR_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR2_MASK   0xc
#define PCIE_RC_CFG_PRIV1_ID_VAL3_CLASS_CODE_MASK       0xffffff

#define PCIE_MISC_MISC_CTRL_SCB_ACCESS_EN_MASK          0x1000
#define PCIE_MISC_MISC_CTRL_CFG_READ_UR_MODE_MASK       0x2000
#define PCIE_MISC_MISC_CTRL_MAX_BURST_SIZE_MASK         0x300000
#define PCIE_MISC_MISC_CTRL_SCB0_SIZE_MASK              0xf8000000
#define PCIE_MISC_MISC_CTRL_SCB1_SIZE_MASK              0x7c00000
#define PCIE_MISC_MISC_CTRL_SCB2_SIZE_MASK              0x1f
#define PCIE_MISC_RC_BAR2_CONFIG_LO_SIZE_MASK           0x1f

#define PCIE_RGR1_SW_INIT_1_INIT_MASK      0x2
#define PCIE_RGR1_SW_INIT_1_PERST_MASK     0x1

#define PCIE_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ_MASK     0x08000000

#define PCIE_MISC_HARD_PCIE_HARD_DEBUG_CLKREQ_DEBUG_ENABLE_MASK        0x2

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_LIMIT_MASK 0xfff00000
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_BASE_MASK  0xfff0
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI_BASE_MASK     0xff
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI_LIMIT_MASK   0xff
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_MASK_BITS             0xc


#define PCIE_MISC_REVISION_MAJMIN_MASK     0xffff

#define BURST_SIZE_128          0
#define BURST_SIZE_256          1
#define BURST_SIZE_512          2

#define PCI_EXP_LNKCAP      12  /* Link Capabilities */
#define  PCI_EXP_LNKCAP_SLS 0x0000000f /* Supported Link Speeds */
#define  PCI_EXP_LNKCAP_SLS_2_5GB 0x00000001 /* LNKCAP2 SLS Vector bit 0 */
#define  PCI_EXP_LNKCAP_SLS_5_0GB 0x00000002 /* LNKCAP2 SLS Vector bit 1 */
#define  PCI_EXP_LNKCAP_SLS_8_0GB 0x00000003 /* LNKCAP2 SLS Vector bit 2 */
#define  PCI_EXP_LNKCAP_SLS_16_0GB 0x00000004 /* LNKCAP2 SLS Vector bit 3 */
#define  PCI_EXP_LNKCAP_SLS_32_0GB 0x00000005 /* LNKCAP2 SLS Vector bit 4 */
#define  PCI_EXP_LNKCAP_MLW 0x000003f0 /* Maximum Link Width */
#define  PCI_EXP_LNKCAP_ASPMS   0x00000c00 /* ASPM Support */
#define  PCI_EXP_LNKCAP_L0SEL   0x00007000 /* L0s Exit Latency */
#define  PCI_EXP_LNKCAP_L1EL    0x00038000 /* L1 Exit Latency */
#define  PCI_EXP_LNKCAP_CLKPM   0x00040000 /* Clock Power Management */
#define  PCI_EXP_LNKCAP_SDERC   0x00080000 /* Surprise Down Error Reporting Capable */
#define  PCI_EXP_LNKCAP_DLLLARC 0x00100000 /* Data Link Layer Link Active Reporting Capable */
#define  PCI_EXP_LNKCAP_LBNC    0x00200000 /* Link Bandwidth Notification Capability */
#define  PCI_EXP_LNKCAP_PN  0xff000000 /* Port Number */
#define PCI_EXP_LNKCTL      16  /* Link Control */
#define  PCI_EXP_LNKCTL_ASPMC   0x0003  /* ASPM Control */
#define  PCI_EXP_LNKCTL_ASPM_L0S 0x0001 /* L0s Enable */
#define  PCI_EXP_LNKCTL_ASPM_L1  0x0002 /* L1 Enable */
#define  PCI_EXP_LNKCTL_RCB 0x0008  /* Read Completion Boundary */
#define  PCI_EXP_LNKCTL_LD  0x0010  /* Link Disable */
#define  PCI_EXP_LNKCTL_RL  0x0020  /* Retrain Link */
#define  PCI_EXP_LNKCTL_CCC 0x0040  /* Common Clock Configuration */
#define  PCI_EXP_LNKCTL_ES  0x0080  /* Extended Synch */
#define  PCI_EXP_LNKCTL_CLKREQ_EN 0x0100 /* Enable clkreq */
#define  PCI_EXP_LNKCTL_HAWD    0x0200  /* Hardware Autonomous Width Disable */
#define  PCI_EXP_LNKCTL_LBMIE   0x0400  /* Link Bandwidth Management Interrupt Enable */
#define  PCI_EXP_LNKCTL_LABIE   0x0800  /* Link Autonomous Bandwidth Interrupt Enable */
#define PCI_EXP_LNKSTA      18  /* Link Status */
#define  PCI_EXP_LNKSTA_CLS 0x000f  /* Current Link Speed */
#define  PCI_EXP_LNKSTA_CLS_2_5GB 0x0001 /* Current Link Speed 2.5GT/s */
#define  PCI_EXP_LNKSTA_CLS_5_0GB 0x0002 /* Current Link Speed 5.0GT/s */
#define  PCI_EXP_LNKSTA_CLS_8_0GB 0x0003 /* Current Link Speed 8.0GT/s */
#define  PCI_EXP_LNKSTA_CLS_16_0GB 0x0004 /* Current Link Speed 16.0GT/s */
#define  PCI_EXP_LNKSTA_CLS_32_0GB 0x0005 /* Current Link Speed 32.0GT/s */
#define  PCI_EXP_LNKSTA_NLW 0x03f0  /* Negotiated Link Width */
#define  PCI_EXP_LNKSTA_NLW_X1  0x0010  /* Current Link Width x1 */
#define  PCI_EXP_LNKSTA_NLW_X2  0x0020  /* Current Link Width x2 */
#define  PCI_EXP_LNKSTA_NLW_X4  0x0040  /* Current Link Width x4 */
#define  PCI_EXP_LNKSTA_NLW_X8  0x0080  /* Current Link Width x8 */
#define  PCI_EXP_LNKSTA_NLW_SHIFT 4 /* start of NLW mask in link status */
#define  PCI_EXP_LNKSTA_LT  0x0800  /* Link Training */
#define  PCI_EXP_LNKSTA_SLC 0x1000  /* Slot Clock Configuration */
#define  PCI_EXP_LNKSTA_DLLLA   0x2000  /* Data Link Layer Link Active */
#define  PCI_EXP_LNKSTA_LBMS    0x4000  /* Link Bandwidth Management Status */
#define  PCI_EXP_LNKSTA_LABS    0x8000  /* Link Autonomous Bandwidth Status */

#endif /* __BCM2711_H__ */
