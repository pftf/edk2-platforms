/** @file
 *
 *  Copyright (c) 2019 Linaro, Limited. All rights reserved.
 *  Copyright (c) 2019 Andrei Warkentin <andrey.warkentin@gmail.com>
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <IndustryStandard/Bcm2711.h>

/*
 * According to UEFI boot log for the VLI device on Pi 4.
 */
#define XHCI_REG_LENGTH 0x1000

Device (XHC0)
{
    Name (_HID, "PNP0D10")      // _HID: Hardware ID
    Name (_UID, 0x0)            // _UID: Unique ID
    Name (_CCA, 0x0)            // _CCA: Cache Coherency Attribute

    Method (_CRS, 0, Serialized) { // _CRS: Current Resource Settings
        Name (RBUF, ResourceTemplate () {
            QWordMemory(ResourceConsumer,
                ,
                MinFixed,
                MaxFixed,
                NonCacheable,
                ReadWrite,
                0x0,
                0xAAAA, // MIN
                0xBBBA, // MAX
                0x0,
                0x1111, // LEN
                ,
                ,
                MMIO
                )
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, ) {
                175
            }
        })
        CreateQwordField (RBUF, MMIO._MIN, MMBA)
        CreateQwordField (RBUF, MMIO._MAX, MMBE)
        CreateQwordField (RBUF, MMIO._LEN, MMLE)
        Store (PCIE_CPU_MMIO_WINDOW, MMBA)
        Store (PCIE_CPU_MMIO_WINDOW, MMBE)
        Store (XHCI_REG_LENGTH, MMLE)
        Add (MMBA, MMLE, MMBE)
        Return (RBUF)
    }

    OperationRegion (PCFG, SystemMemory, PCIE_REG_BASE + PCIE_EXT_CFG_DATA, 0x1000)
    Field (PCFG, AnyAcc, NoLock, Preserve) {
        Offset (0),
        VNID, 16, // Vendor ID
        DVID, 16, // Device ID
        CMND, 16, // Command register
        STAT, 16, // Status register
    }

    Method (_INI) {
        // Start with a known-good state.
        _PS3 ()
    }

    Method (_PS0) {
        // Set command register to:
        // 1) decode MMIO (set bit 1)
        // 2) enable DMA (set bit 2)
        // 3) enable interrupts (clear bit 10)
        Debug = "xHCI to D0 (enable)"
        Store (0x6,CMND)
    }

    Method (_PS3) {
        // Set command register to:
        // 1) not decode MMIO (clear bit 1)
        // 2) disable DMA (clear bit 2)
        // 3) disable interrupts (set bit 10)
        Debug = "xHCI to D3 (disable)"
        Store (0x400,CMND)
    }

    Method (_PSC) {
        If (LEqual (CMND, 0x6)) {
           Debug = "xHCI is in D0 (enabled)"
           Return (0)
        }

        if (LNotEqual (CMND, 0x400)) {
           Debug = "xHCI is not in D0, but not everything is off"
           DEBUG = VNID
           DEBUG = DVID
           DEBUG = CMND
           DEBUG = STAT
        }
        Return (3)
    }

    Method (_STA) {
        If (LEqual (CMND, 0x6)) {
           Debug = "xHCI reporting _STA as present/enabled/UI/working"
           Return (0xf)
        }

        if (LNotEqual (CMND, 0x400)) {
           Debug = "xHCI not cleanly initialized"
           DEBUG = VNID
           DEBUG = DVID
           DEBUG = CMND
           DEBUG = STAT
        }

        Debug = "xHCI reporting _STA as present"
        Return (0x1)
    }
}
