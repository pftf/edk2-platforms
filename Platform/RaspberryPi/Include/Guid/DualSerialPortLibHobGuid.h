/** @file
*
*  Copyright (c) 2020, Pete Batard <pete@akeo.ie>
*  Copyright (c) 2020, Andrei Warkentin <andrey.warkentin@gmail.com>
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#ifndef _DUAL_SERIAL_PORT_LIB_HOB_GUID_H_
#define _DUAL_SERIAL_PORT_LIB_HOB_GUID_H_

#define DUAL_SERIAL_PORT_LIB_HOB_GUID \
  { 0x1CB0EB5B, 0x4F73, 0x4308, { 0xA3, 0x33, 0x1D, 0x95, 0xBE, 0x5F, 0x30, 0xB5 } }

extern GUID gDualSerialPortLibHobGuid;

typedef struct {
  UINT32 CoreClockFreq;
} DUAL_SERIAL_PORT_LIB_VARS;

#endif /* _DUAL_SERIAL_PORT_LIB_HOB_GUID_H_ */
