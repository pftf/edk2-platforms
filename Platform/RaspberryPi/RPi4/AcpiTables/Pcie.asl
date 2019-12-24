/** @file
*  Secondary System Description Table (SSDT)
*
*  Copyright (c) 2018, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include "AcpiTables.h"

DefinitionBlock("SsdtPci.aml", "SSDT", 2, "MCRSFT", "FAKEPCIE", EFI_ACPI_OEM_REVISION) {
  Scope (_SB) {

#include <IndustryStandard/Bcm2711.h>

/*
 * The following can be used to remove parenthesis from
 * defined macros that the compiler complains about.
 */
#define _REMOVE_PAREN(...)      __VA_ARGS__
#define REMOVE_PAREN(x)         _REMOVE_PAREN x

#define ROOT_PRT_ENTRY(Pin, Link)   PRT_ENTRY(0x0000FFFF, Pin, Link)

#define LNK_DEVICE(Unique_Id, Link_Name, irq)							\
	Device(Link_Name) {									\
	    Name(_HID, EISAID("PNP0C0F"))							\
	    Name(_UID, Unique_Id)								\
	    Name(_PRS, ResourceTemplate() {							\
	        Interrupt(ResourceProducer, Level, ActiveHigh, Exclusive) { irq }		\
	    })											\
	    Method (_CRS, 0) { Return (_PRS) }							\
	    Method (_SRS, 1) { }								\
	    Method (_DIS) { }									\
	}

#define PRT_ENTRY(Address, Pin, Link)								  \
        Package (4) {                                                                             \
            Address,    /* uses the same format as _ADR */                                        \
            Pin,        /* The PCI pin number of the device (0-INTA, 1-INTB, 2-INTC, 3-INTD). */  \
            Link,       /* Interrupt allocated via Link device. */   	     	     	      	  \
            Zero        /* global system interrupt number (no used) */				  \
          }

    LNK_DEVICE(1, LNKA, 175)
        
    // PCI Root Complex
    Device (PCI0) {
      Name (_HID, EISAID("PNP0A08")) // PCI Express Root Bridge
      Name (_CID, EISAID("PNP0A03")) // Compatible PCI Root Bridge
      Name (_SEG, Zero)              // PCI Segment Group number
      Name (_BBN, Zero)              // PCI Base Bus Number
      Name (_CCA, 0)                 // Cache Coherency Attribute

        Method (_INI, 0, Serialized) {
            OperationRegion (PCFG, SystemMemory, REMOVE_PAREN(PCIE_REG_BASE) + PCIE_EXT_CFG_DATA, 0x1000)
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

   // Root complex resources
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (      // Bus numbers assigned to this root
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            0,                 // AddressGranularity
            0,                 // AddressMinimum - Minimum Bus Number
            0,                 // AddressMaximum - Maximum Bus Number
            0,                 // AddressTranslation - Set to 0
            1                  // RangeLength - Number of Busses
          )

          /* QWordMemory ( */
          /*   ResourceProducer, */
          /*   PosDecode, */
          /*   MinFixed, */
          /*   MaxFixed, */
          /*   NonCacheable, */
          /*   ReadWrite, */
          /*   0x00000000, // Granularity */
          /*   0x00000000, // MIN */
          /*   0xffffffff, // MAX */
          /*   0x600000000,// TRA */
          /*   0x100000000, // LEN */
          /* ) */

                      QWordMemory (
            ResourceProducer,
            PosDecode,
            MinFixed,
            MaxFixed,
            NonCacheable,
            ReadWrite,
            0x00000000, // Granularity
            0xf8000000, // MIN
            0x1f7ffffff, // MAX
            0x508000000,// TRA
            0x100000000, // LEN
          )
        }) // Name (RBUF)

        Return (RBUF)
      } // Method (_CRS)

    Method(_DSM, 0x4, NotSerialized) {
      If (LEqual(Arg0, ToUUID("E5C937D0-3553-4d7a-9117-EA4D19C3434D"))) {
        switch (ToInteger(Arg2)) {
          //
          // Function 0: Return supported functions
        case(0) {
          Return (Buffer() {0xFF})
            }
        
        //
        // Function 5: Return Ignore PCI Boot Configuration
        //
        case(5) {
          Return (Package(1) {0})
            }

        //
        // Not supported
        //
        default {
        }
        }
      }
      Return (Buffer(){0})
        } // Method(_DSM)

      Device (DEV0) {
        Name (_ADR, 0xf0000000)
      }

      // PCI Routing Table
      Name(_PRT, Package() {
        ROOT_PRT_ENTRY(0, LNKA),   // INTA
      })
    }
  }
}
