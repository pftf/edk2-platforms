/** @file
 *
 *  Copyright (c) 2019 Linaro, Limited. All rights reserved.
 *  Copyright (c) 2020 Arm
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <IndustryStandard/Bcm2711.h>

#include "AcpiTables.h"

/*
 * The following can be used to remove parenthesis from
 * defined macros that the compiler complains about.
 */
#define ISOLATE_ARGS(...)               __VA_ARGS__
#define REMOVE_PARENTHESES(x)           ISOLATE_ARGS x

#define SANITIZED_PCIE_CPU_MMIO_WINDOW  REMOVE_PARENTHESES(PCIE_CPU_MMIO_WINDOW)                        // 600000000
#define SANITIZED_PCIE_MMIO_LEN         REMOVE_PARENTHESES(PCIE_BRIDGE_MMIO_LEN)                        //  03ffffff
#define SANITIZED_PCIE_PCI_MMIO_BEGIN   REMOVE_PARENTHESES(PCIE_TOP_OF_MEM_WIN)                         //  f8000000

/*
 * According to UEFI boot log for the VLI device on Pi 4.
 */
#define XHCI_REG_LENGTH                 0x1000

#define LNK_DEVICE(Unique_Id, Link_Name, irq)             \
  Device(Link_Name) {                 \
      Name(_HID, EISAID("PNP0C0F"))             \
      Name(_UID, Unique_Id)               \
      Name(_PRS, ResourceTemplate() {             \
          Interrupt(ResourceProducer, Level, ActiveHigh, Exclusive) { irq }   \
      })                      \
      Method (_CRS, 0) { Return (_PRS) }              \
      Method (_SRS, 1) { }                \
      Method (_DIS) { }                 \
  }

#define PRT_ENTRY(Address, Pin, Link)                 \
        Package (4) {                                                                             \
            Address,    /* uses the same format as _ADR */                                        \
            Pin,        /* The PCI pin number of the device (0-INTA, 1-INTB, 2-INTC, 3-INTD). */  \
            Link,       /* Interrupt allocated via Link device. */                          \
            Zero        /* global system interrupt number (no used) */          \
          }
#define ROOT_PRT_ENTRY(Pin, Link)   PRT_ENTRY(0x0000FFFF, Pin, Link)

DefinitionBlock (__FILE__, "SSDT", 5, "RPIFDN", "RPI4PCIE", 2)
{
  Scope (\_SB_)
  {

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
            QWordMemory (ResourceProducer,
                ,
                MinFixed,
                MaxFixed,
                NonCacheable,
                ReadWrite,
                0x0,
                SANITIZED_PCIE_CPU_MMIO_WINDOW, // MIN
                SANITIZED_PCIE_CPU_MMIO_WINDOW, // MAX
                0x0,
                0x1,                            // LEN
                ,
                ,
                MMIO
                )
        })
        CreateQwordField (RBUF, MMIO._MAX, MMBE)
        CreateQwordField (RBUF, MMIO._LEN, MMLE)
        Add (MMBE, XHCI_REG_LENGTH - 1, MMBE)
        Add (MMLE, XHCI_REG_LENGTH - 1, MMLE)
        Return (RBUF)
      }

      Name (_DMA, ResourceTemplate() {
        /*
         * PCIe is limited to DMA to first 3GB. Note this
         * only applies to PCIe, not GENET or other devices
         */
        QWordMemory (ResourceConsumer,
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

      //
      // PCI Root Complex
      //
      LNK_DEVICE(1, LNKA, 175)
      LNK_DEVICE(2, LNKB, 176) //TODO: verify int b,c,d values
      LNK_DEVICE(3, LNKC, 177)
      LNK_DEVICE(4, LNKD, 178)

      Device(PCI0)
      {
        Name(_HID, EISAID("PNP0A08")) // PCI Express Root Bridge
        Name(_CID, EISAID("PNP0A03")) // Compatible PCI Root Bridge
        Name(_SEG, Zero) // PCI Segment Group number
        Name(_BBN, Zero) // PCI Base Bus Number
        Name(_CCA, 0)    // mark the PCI noncoherent

        // Root Complex 0
        Device (RP0) {
         Name(_ADR, 0xF0000000)    // Dev 0, Func 0
        }

        Name (_DMA, ResourceTemplate() {
          QWordMemory (ResourceConsumer,
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

        // PCI Routing Table
        Name(_PRT, Package() {
          ROOT_PRT_ENTRY(0, LNKA),   // INTA
          ROOT_PRT_ENTRY(1, LNKB),   // INTB
          ROOT_PRT_ENTRY(2, LNKC),   // INTC
          ROOT_PRT_ENTRY(3, LNKD),   // INTD
        })
        // Root complex resources
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            WordBusNumber ( // Bus numbers assigned to this root
              ResourceProducer,
              MinFixed, MaxFixed, PosDecode,
              0,   // AddressGranularity
              0,   // AddressMinimum - Minimum Bus Number
              255, // AddressMaximum - Maximum Bus Number
              0,   // AddressTranslation - Set to 0
              256  // RangeLength - Number of Busses
            )

            QWordMemory ( // 32-bit BAR Windows in 64-bit addr
              ResourceProducer, PosDecode,
              MinFixed, MaxFixed,
              NonCacheable, ReadWrite,        //cacheable? is that right?
              0x00000000,                     // Granularity
              0,                              // SANITIZED_PCIE_PCI_MMIO_BEGIN
              1,                              // SANITIZED_PCIE_MMIO_LEN + SANITIZED_PCIE_PCI_MMIO_BEGIN
              SANITIZED_PCIE_CPU_MMIO_WINDOW, // SANITIZED_PCIE_PCI_MMIO_BEGIN - SANITIZED_PCIE_CPU_MMIO_WINDOW
              2                               // SANITIZED_PCIE_MMIO_LEN + 1
              ,,,MMI1,,TypeTranslation
            )
          }) // Name(RBUF)

          // Work around ASL's inability to add in a resource definition
          // or for that matter compute the min,max,len properly
          CreateQwordField (RBUF, MMI1._MIN, MMIB)
          CreateQwordField (RBUF, MMI1._MAX, MMIE)
          CreateQwordField (RBUF, MMI1._TRA, MMIT)
          CreateQwordField (RBUF, MMI1._LEN, MMIL)
          Add (MMIB, SANITIZED_PCIE_PCI_MMIO_BEGIN, MMIB)
          Add (SANITIZED_PCIE_MMIO_LEN, SANITIZED_PCIE_PCI_MMIO_BEGIN, MMIE)
          Subtract (MMIT, SANITIZED_PCIE_PCI_MMIO_BEGIN, MMIT)
          Add (SANITIZED_PCIE_MMIO_LEN, 1 , MMIL)

          Return (RBUF)
        } // Method(_CRS)
        //
        // OS Control Handoff
        //
        Name(SUPP, Zero) // PCI _OSC Support Field value
        Name(CTRL, Zero) // PCI _OSC Control Field value

        // See [1] 6.2.10, [2] 4.5
        Method(_OSC,4) {
          // Note, This code is very similar to the code in the PCIe firmware
          // specification which can be used as a reference
          // Check for proper UUID
          If(LEqual(Arg0,ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {
            // Create DWord-adressable fields from the Capabilities Buffer
            CreateDWordField(Arg3,0,CDW1)
            CreateDWordField(Arg3,4,CDW2)
            CreateDWordField(Arg3,8,CDW3)

            // Save Capabilities DWord2 & 3
            Store(CDW2,SUPP)
            Store(CDW3,CTRL)

            // Mask out Native HotPlug
            And(CTRL,0x1E,CTRL)
            // Always allow native PME, AER (no dependencies)

            // Never allow SHPC (no SHPC controller in this system)
            And(CTRL,0x1D,CTRL)

            If(LNotEqual(Arg1,One)) { // Unknown revision
              Or(CDW1,0x08,CDW1)
            }

            If(LNotEqual(CDW3,CTRL)) {  // Capabilities bits were masked
              Or(CDW1,0x10,CDW1)
            }
            // Update DWORD3 in the buffer
            Store(CTRL,CDW3)
            Return(Arg3)
          } Else {
            Or(CDW1,4,CDW1) // Unrecognized UUID
            Return(Arg3)
          }
        } // End _OSC
      } // PCI0
    } //end SCB0
  } //end scope sb
} //end definition block
