/** @file
 *
 *  [DSDT] Serial devices (UART).
 *
 *  Copyright (c) 2018, Andrey Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

// PL011 based UART.
Device (URT0)
{
  Name (_HID, "BCM2837")
  Name (_CID, "ARMH0011")
  Name (_UID, 0x4)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return (0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate ()
    {
      MEMORY32FIXED (ReadWrite, 0xFE201000, 0x1000,)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x99 }
    })
    Return (RBUF)
  }

  Name (CLCK, 48000000)

  Name (_DSD, Package ()
  {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"), Package ()
    {
      Package (2) { "clock-frequency", CLCK },
    }
  })
}
