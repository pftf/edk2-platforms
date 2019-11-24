/** @file
 *
 *  Copyright (c) 2019 Linaro, Limited. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <IndustryStandard/Bcm2711.h>

Device (XHC0)
{
    Name (_HID, "PNP0D10")      // _HID: Hardware ID
    Name (_UID, 0x0)            // _UID: Unique ID
    Name (_CCA, 0x0)            // _CCA: Cache Coherency Attribute

    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
        QWordMemory(ResourceConsumer,
            ,
            MinFixed,
            MaxFixed,
            NonCacheable,
            ReadWrite,
            0x0,
            0x600000000,
            0x600003fff,
            0x0,
            0x4000
            )
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
        {
          175
        }
    })
    OperationRegion (PCFG, SystemMemory, PCIE_REG_BASE, 0x10000)
    Field (PCFG, AnyAcc, NoLock, Preserve) {
        Offset (PCIE_EXT_CFG_DATA),
        VDID, 32,
        CMND, 3,
    }
    Method (_STA, 0x0, Serialized) {
      Store (0x7, CMND) // enable memdecode and busmastering
      Return (0xf)
    }
}
