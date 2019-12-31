/** @file
 *
 *  [DSDT] Devices behind the GPU.
 *
 *  Copyright (c) 2018-2019, Andrey Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

// DWC OTG Controller
Device (USB0)
{
  Name (_HID, "BCM2848")
  Name (_CID, Package() { "DWC_OTG", "DWC2_OTG"})
  Name (_UID, 0x0)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return (0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate ()
    {
      MEMORY32FIXED(ReadWrite, 0xFE980000, 0x10000,)

      // Validated on Pi 4
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x69 }
    })
    Return(RBUF)
  }
}

//
// Video Core 4 GPU
//
// This Pi3 definition cannot be used on Pi4 until
// the memory/interrupt ranges are validated.
//
// Device (GPU0)
// {
//   Name (_HID, "BCM2850")
//   Name (_CID, "VC4")
//   Name (_UID, 0x0)
//   Name (_CCA, 0x0)
//   Method (_STA)
//   {
//     Return(0xf)
//   }
//   Method (_CRS, 0x0, Serialized)
//   {
//     Name (RBUF, ResourceTemplate ()
//     {
//       // Memory and interrupt for the GPU.
// 
//       // FIXME: interrupts wrong for Pi 4.
// 
//       MEMORY32FIXED(ReadWrite, 0xFEC00000, 0x1000,)
//       Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x2A }
// 
//       // HVS - Hardware Video Scalar
//       MEMORY32FIXED (ReadWrite, 0xFE400000, 0x6000,)
//       // The HVS interrupt is reserved by the VPU
//       // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x41 }
// 
//       // PixelValve0 - DSI0 or DPI
//       // MEMORY32FIXED (ReadWrite, 0xFE206000, 0x100,)
//       // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x4D }
// 
//       // PixelValve1 - DS1 or SMI
//       // MEMORY32FIXED (ReadWrite, 0xFE207000, 0x100,)
//       // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x4E }
// 
//       // PixelValve2 - HDMI output - connected to HVS display FIFO 1
//       MEMORY32FIXED (ReadWrite, 0xFE807000, 0x100,)
//       Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x4A }
// 
//       // HDMI registers
//       MEMORY32FIXED (ReadWrite, 0xFE902000, 0x600,)   // HDMI registers
//       MEMORY32FIXED (ReadWrite, 0xFE808000, 0x100,)   // HD registers
//       // hdmi_int[0]
//       // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x48 }
//       // hdmi_int[1]
//       // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x49 }
// 
//       // HDMI DDC connection
//       I2CSerialBus (0x50,, 100000,, "\\_SB.GDV0.I2C2",,,,)  // EDID
//       I2CSerialBus (0x30,, 100000,, "\\_SB.GDV0.I2C2",,,,)  // E-DDC Segment Pointer
//     })
//     Return(RBUF)
//   }
// 
//   // GPU Power Management Component Data
//   // Reference : https://github.com/Microsoft/graphics-driver-samples/wiki/Install-Driver-in-a-Windows-VM
//   Method (PMCD, 0, Serialized)
//   {
//     Name (RBUF, Package ()
//     {
//       1,                  // Version
//       1,                  // Number of graphics power components
//       Package ()          // Power components package
//       {
//         Package ()        // GPU component package
//         {
//           0,              // Component Index
//           0,              // DXGK_POWER_COMPONENT_MAPPING.ComponentType (0 = DXGK_POWER_COMPONENT_ENGINE)
//           0,              // DXGK_POWER_COMPONENT_MAPPING.NodeIndex
// 
//           Buffer ()       // DXGK_POWER_RUNTIME_COMPONENT.ComponentGuid
//           {               // 9B2D1E26-1575-4747-8FC0-B9EB4BAA2D2B
//             0x26, 0x1E, 0x2D, 0x9B, 0x75, 0x15, 0x47, 0x47,
//             0x8f, 0xc0, 0xb9, 0xeb, 0x4b, 0xaa, 0x2d, 0x2b
//           },
// 
//           "VC4_Engine_00",// DXGK_POWER_RUNTIME_COMPONENT.ComponentName
//           2,              // DXGK_POWER_RUNTIME_COMPONENT.StateCount
// 
//           Package ()      // DXGK_POWER_RUNTIME_COMPONENT.States[] package
//           {
//             Package ()   // F0
//             {
//               0,         // DXGK_POWER_RUNTIME_STATE.TransitionLatency
//               0,         // DXGK_POWER_RUNTIME_STATE.ResidencyRequirement
//               1210000,   // DXGK_POWER_RUNTIME_STATE.NominalPower (microwatt)
//             },
// 
//             Package ()   // F1 - Placeholder
//             {
//               10000,     // DXGK_POWER_RUNTIME_STATE.TransitionLatency
//               10000,     // DXGK_POWER_RUNTIME_STATE.ResidencyRequirement
//               4,         // DXGK_POWER_RUNTIME_STATE.NominalPower
//             },
//           }
//         }
//       }
//     })
//     Return (RBUF)
//   }
// }

// PiQ Mailbox Driver
Device (RPIQ)
{
  Name (_HID, "BCM2849")
  Name (_CID, "RPIQ")
  Name (_UID, 0)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return (0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0xFE00B880, 0x00000024,)

      // Validated on Pi 4
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x41 }
    })
    Return (RBUF)
  }
}

// VCHIQ Driver
Device (VCIQ)
{
  Name (_HID, "BCM2835")
  Name (_CID, "VCIQ")
  Name (_UID, 0)
  Name (_CCA, 0x0)
  Name (_DEP, Package() { \_SB.GDV0.RPIQ })
  Method (_STA)
  {
     Return (0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0xFE00B840, 0x00000010,)

      // Validated on Pi 4
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x42 }
    })
    Return (RBUF)
  }
}

// VC Shared Memory Driver
Device (VCSM)
{
  Name (_HID, "BCM2856")
  Name (_CID, "VCSM")
  Name (_UID, 0)
  Name (_CCA, 0x0)
  Name (_DEP, Package() { \_SB.GDV0.VCIQ })
  Method (_STA)
  {
    Return (0xf)
  }
}

// Description: GPIO
Device (GPI0)
{
  Name (_HID, "BCM2845")
  Name (_CID, "BCMGPIO")
  Name (_UID, 0x0)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return(0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate ()
    {
      MEMORY32FIXED (ReadWrite, 0xFE200000, 0xB4, )

      // Validated on Pi 4
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x91 }
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x92 }
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x93 }
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x94 }
    })
    Return (RBUF)
  }
}

// Description: I2C
Device (I2C1)
{
  Name (_HID, "BCM2841")
  Name (_CID, "BCMI2C")
  Name (_UID, 0x1)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return(0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate ()
    {
      Memory32Fixed(ReadWrite, 0xFE804000, 0x20)

      // Validated on Pi 4
      Interrupt(ResourceConsumer, Level, ActiveHigh, Shared) {0x95}

      PinFunction (Exclusive, PullUp, BCM_ALT0, "\\_SB.GDV0.GPI0", 0, ResourceConsumer,) {2, 3}
    })
    Return (RBUF)
  }
}

// I2C2 is the HDMI DDC connection
Device (I2C2)
{
  Name (_HID, "BCM2841")
  Name (_CID, "BCMI2C")
  Name (_UID, 0x2)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return (0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate()
    {
      Memory32Fixed (ReadWrite, 0xFE805000, 0x20)

      // Validated on Pi 4
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) {0x95}
    })
    Return (RBUF)
  }
}

// SPI
Device (SPI0)
{
  Name (_HID, "BCM2838")
  Name (_CID, "BCMSPI0")
  Name (_UID, 0x0)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return (0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate ()
    {
      MEMORY32FIXED (ReadWrite, 0xFE204000, 0x20,)

      // Validated on Pi 4
      Interrupt(ResourceConsumer, Level, ActiveHigh, Shared) {0x96}

      PinFunction (Exclusive, PullDown, BCM_ALT0, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, ) {9, 10, 11} // MISO, MOSI, SCLK
      PinFunction (Exclusive, PullUp, BCM_ALT0, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, ) {8}     // CE0
      PinFunction (Exclusive, PullUp, BCM_ALT0, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, ) {7}     // CE1
    })
    Return (RBUF)
  }
}

Device (SPI1)
{
  Name (_HID, "BCM2839")
  Name (_CID, "BCMAUXSPI")
  Name (_UID, 0x1)
  Name (_CCA, 0x0)
  Name (_DEP, Package() { \_SB.GDV0.RPIQ })
  Method (_STA)
  {
    Return (0xf)
  }
  Method (_CRS, 0x0, Serialized)
  {
    Name (RBUF, ResourceTemplate ()
    {
      MEMORY32FIXED (ReadWrite, 0xFE215080, 0x40,)

      // Validated on Pi 4
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared,) {0x7D}

      PinFunction(Exclusive, PullDown, BCM_ALT4, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, ) {19, 20, 21} // MISO, MOSI, SCLK
      PinFunction(Exclusive, PullDown, BCM_ALT4, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, ) {16} // CE2
    })
    Return (RBUF)
  }
}

// SPI2 has no pins on GPIO header
// Device (SPI2)
// {
//   Name (_HID, "BCM2839")
//   Name (_CID, "BCMAUXSPI")
//   Name (_UID, 0x2)
//   Name (_CCA, 0x0)
//   Name (_DEP, Package() { \_SB.GDV0.RPIQ })
//   Method (_STA)
//   {
//     Return (0xf)     // Disabled
//   }
//   Method (_CRS, 0x0, Serialized)
//   {
//     Name (RBUF, ResourceTemplate ()
//     {
//       MEMORY32FIXED (ReadWrite, 0xFE2150C0, 0x40,)
//
//       // Validated on Pi 4
//       Interrupt (ResourceConsumer, Level, ActiveHigh, Shared,) {0x7D}
//     })
//     Return (RBUF)
//   }
// }

//
// PWM Driver
//
// This Pi3 definition cannot be used on Pi 4, because
// the consumed resources contain a mix of CPU and bus
// addresses, which is hopelessly invalid.
//
// Device (PWM0)
// {
//   Name (_HID, "BCM2844")
//   Name (_CID, "BCM2844")
//   Name (_UID, 0)
//   Name (_CCA, 0x0)
//   Method (_STA)
//   {
//     Return (0xf)
//   }
//   Method (_CRS, 0x0, Serialized)
//   {
//     Name (RBUF, ResourceTemplate ()
//     {
//       // DMA channel 11 control
//       Memory32Fixed (ReadWrite, 0xFE007B00, 0x00000100,)
//       // PWM control
//       Memory32Fixed (ReadWrite, 0xFE20C000, 0x00000028,)
//       // PWM control bus
//       Memory32Fixed (ReadWrite, 0x7E20C000, 0x00000028,)
//       // PWM control uncached
//       Memory32Fixed (ReadWrite, 0xFF20C000, 0x00000028,)
//       // PWM clock control
//       Memory32Fixed (ReadWrite, 0xFE1010A0, 0x00000008,)
//       // Interrupt DMA channel 11
// 
//       // FIXME: interrupts wrong for Pi 4.
//       Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x3B }
//       // DMA channel 11, DREQ 5 for PWM
//       FixedDMA (5, 11, Width32Bit, )
//     })
//     Return (RBUF)
//   }
// }

include ("Uart.asl")
include ("Rhpx.asl")
include ("Sdhc.asl")