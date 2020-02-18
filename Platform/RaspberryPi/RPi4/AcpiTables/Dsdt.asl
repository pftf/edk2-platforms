/** @file
 *
 *  Differentiated System Definition Table (DSDT)
 *
 *  Copyright (c) 2018, Andrey Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

DefinitionBlock ("Dsdt.aml", "DSDT", 5, "MSFT", "EDK2", 2)
{
  Scope (\_SB_)
  {
    include ("Xhci.asl")

    // DWC OTG Controller
    Device (USB0) {
       Name (_HID, "BCM2848")
       Name (_CID, Package() { "DWC_OTG", "DWC2_OTG"})
       Name (_UID, 0x0)
       Name (_CCA, 0x0)
       Method (_STA) {
          Return (0xf)
       }
       Method (_CRS, 0x0, Serialized) {
          Name (RBUF, ResourceTemplate () {
             MEMORY32FIXED(ReadWrite, 0xFE980000, 0x10000,)
             Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x69 }
          })
          Return(RBUF)
       }
    }

    Device (CPU0)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x0)
      Method (_STA)
      {
        Return (0xf)
      }
    }

    Device (CPU1)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x1)
      Method (_STA)
      {
        Return (0xf)
      }
    }

    Device (CPU2)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x2)
      Method (_STA)
      {
        Return (0xf)
      }
    }

    Device (CPU3)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x3)
      Method (_STA)
      {
        Return (0xf)
      }
    }

    include ("Uart.asl")
  }
}
