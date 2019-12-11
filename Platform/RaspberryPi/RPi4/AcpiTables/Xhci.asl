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

Device (SCB0) {
    Name (_HID, "ACPI0004")
    Name (_UID, 0x0)
    Name (_CCA, 0x0)

    Method (_CRS, 0, Serialized) { // _CRS: Current Resource Settings
        /*
         * Container devices with _DMA must have _CRS, meaning SCB0
         * to provide all resources that XHC0 consumes (except
         * interrupts).
         */
        Name (RBUF, ResourceTemplate () {
            QWordMemory(ResourceProducer,
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

    Name (_DMA, ResourceTemplate() {
        /*
         * XHC0 is limited to DMA to first 3GB. Note this
         * only applies to PCIe, not GENET or other devices
         * next to the A72.
         */
        QWordMemory(ResourceConsumer,
            ,
            MinFixed,
            MaxFixed,
            NonCacheable,
            ReadWrite,
            0x0,
            0x0,        // MIN
            0xbfffffff, // MAX
            0x0,        // TRA
            0xc0000000, // LEN
            ,
            ,
            )
    })

    Device (XHC0)
    {
        Name (_HID, "11063483")     // _HID: Hardware ID
        Name (_CID, "PNP0D10")      // _CID: Hardware ID
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

        Method (_INI, 0, Serialized) {
            OperationRegion (PCFG, SystemMemory, PCIE_REG_BASE + PCIE_EXT_CFG_DATA, 0x1000)
            Field (PCFG, AnyAcc, NoLock, Preserve) {
                Offset (0),
                VNID, 16, // Vendor ID
                DVID, 16, // Device ID
                CMND, 16, // Command register
                STAT, 16, // Status register
            }

            // Set command register to:
            // 1) decode MMIO (set bit 1)
            // 2) enable DMA (set bit 2)
            // 3) enable interrupts (clear bit 10)
            Debug = "xHCI enable"
            Store (0x6, CMND)
        }
    }
}
