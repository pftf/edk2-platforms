/** @file

  Copyright (c) 2020, Pete Batard <pete@akeo.ie>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef BCM_GENET_H__
#define BCM_GENET_H__

#include <Library/PcdLib.h>

#define GENET_BASE_ADDRESS         FixedPcdGet64 (PcdBcmGenetRegistersAddress)
#define GENET_LENGTH               0x00010000

#endif /* BCM_GENET_H__ */
