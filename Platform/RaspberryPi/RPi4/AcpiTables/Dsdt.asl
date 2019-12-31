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

#define BCM_ALT0 0x4
#define BCM_ALT1 0x5
#define BCM_ALT2 0x6
#define BCM_ALT3 0x7
#define BCM_ALT4 0x3
#define BCM_ALT5 0x2

DefinitionBlock ("Dsdt.aml", "DSDT", 5, "MSFT", "EDK2", 2)
{
  Scope (\_SB_)
  {
    include ("Pep.asl")
    include ("Xhci.asl")

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

    //
    // GPU device container describes the DMA translation required when
    // a device behind the GPU wants to access Arm memory. Only the first
    // 1GB can be addressed.
    //
    Device (GDV0)
    {
      Name (_HID, "ACPI0004")
      Name (_UID, 0x1)
      Name (_CCA, 0x0)

      Method (_CRS, 0, Serialized) { // _CRS: Current Resource Settings
        /*
         * Container devices with _DMA must have _CRS, meaning GDV0
         * to provide all resources that GpuDevs.asl consume (except
         * interrupts).
         */
        Name (RBUF, ResourceTemplate () {

          // USB0
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE980000, // MIN
                0xFE98FFFF, // MAX
                0x0,
                0x10000, // LEN
                ,,)

          // VCHIQ
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE00B840, // MIN
                0xFE00B84F, // MAX
                0x0,
                0x10, // LEN
                ,,)

          // GPIO
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE200000, // MIN
                0xFE2000B3, // MAX
                0x0,
                0xB4, // LEN
                ,,)

          // I2C1
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE804000, // MIN
                0xFE80401F, // MAX
                0x0,
                0x20, // LEN
                ,,)

          // I2C2
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE805000, // MIN
                0xFE80501F, // MAX
                0x0,
                0x20, // LEN
                ,,)

          // SPI0
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE204000, // MIN
                0xFE20401F, // MAX
                0x0,
                0x20, // LEN
                ,,)

          // SPI1
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE215080, // MIN
                0xFE2150BF, // MAX
                0x0,
                0x40, // LEN
                ,,)

          // URT0
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE201000, // MIN
                0xFE201FFF, // MAX
                0x0,
                0x1000, // LEN
                ,,)

          // URTM
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE215000, // MIN
                0xFE21506F, // MAX
                0x0,
                0x70, // LEN
                ,,)

          // URTM
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE215000, // MIN
                0xFE21506F, // MAX
                0x0,
                0x70, // LEN
                ,,)

          // SDC1
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE300000, // MIN
                0xFE3000FF, // MAX
                0x0,
                0x100, // LEN
                ,,)

          // SDC2
          QWordMemory(ResourceProducer,,
                MinFixed, MaxFixed, NonCacheable, ReadWrite, 0x0,
                0xFE202000, // MIN
                0xFE2020FF, // MAX
                0x0,
                0x100, // LEN
                ,,)

          })
          Return (RBUF)
       }

       Name (_DMA, ResourceTemplate() {
        /*
         * Only the first 1GB is available.
         * CPU 0x0 -> bus 0xc0000000.
         */
        QWordMemory(ResourceConsumer,
            ,
            MinFixed,
            MaxFixed,
            NonCacheable,
            ReadWrite,
            0x0,
            0xc0000000, // MIN
            0xffffffff, // MAX
            0x40000000, // TRA
            0x40000000, // LEN
            ,
            ,
            )
       })
       include ("GpuDevs.asl")
    }
  }
}
