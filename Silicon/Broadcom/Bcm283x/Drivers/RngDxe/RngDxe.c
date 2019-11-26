/** @file

  This driver produces an EFI_RNG_PROTOCOL instance for the Broadcom 2836 RNG

  Copyright (C) 2019, Pete Batard <pete@akeo.ie>
  Copyright (C) 2019, Linaro Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <IndustryStandard/Bcm2836.h>

#include <Protocol/Rng.h>

#define RNG_WARMUP_COUNT        0x40000
#define RNG_MAX_RETRIES         0x100         // arbitrary upper bound
#define RNG_FIFO_NUMVAL_MASK    0xff
#define RNG_STATUS_NUMVAL_SHIFT 24

/**
  Returns information about the random number generation implementation.

  @param[in]     This                 A pointer to the EFI_RNG_PROTOCOL
                                      instance.
  @param[in,out] RNGAlgorithmListSize On input, the size in bytes of
                                      RNGAlgorithmList.
                                      On output with a return code of
                                      EFI_SUCCESS, the size in bytes of the
                                      data returned in RNGAlgorithmList. On
                                      output with a return code of
                                      EFI_BUFFER_TOO_SMALL, the size of
                                      RNGAlgorithmList required to obtain the
                                      list.
  @param[out] RNGAlgorithmList        A caller-allocated memory buffer filled
                                      by the driver with one EFI_RNG_ALGORITHM
                                      element for each supported RNG algorithm.
                                      The list must not change across multiple
                                      calls to the same driver. The first
                                      algorithm in the list is the default
                                      algorithm for the driver.

  @retval EFI_SUCCESS                 The RNG algorithm list was returned
                                      successfully.
  @retval EFI_UNSUPPORTED             The services is not supported by this
                                      driver.
  @retval EFI_DEVICE_ERROR            The list of algorithms could not be
                                      retrieved due to a hardware or firmware
                                      error.
  @retval EFI_INVALID_PARAMETER       One or more of the parameters are
                                      incorrect.
  @retval EFI_BUFFER_TOO_SMALL        The buffer RNGAlgorithmList is too small
                                      to hold the result.

**/
STATIC
EFI_STATUS
EFIAPI
Bcm2836RngGetInfo (
  IN      EFI_RNG_PROTOCOL        *This,
  IN OUT  UINTN                   *RNGAlgorithmListSize,
  OUT     EFI_RNG_ALGORITHM       *RNGAlgorithmList
  )
{
  if (This == NULL || RNGAlgorithmListSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*RNGAlgorithmListSize < sizeof (EFI_RNG_ALGORITHM)) {
    *RNGAlgorithmListSize = sizeof (EFI_RNG_ALGORITHM);
    return EFI_BUFFER_TOO_SMALL;
  }

  if (RNGAlgorithmList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *RNGAlgorithmListSize = sizeof (EFI_RNG_ALGORITHM);
  CopyGuid (RNGAlgorithmList, &gEfiRngAlgorithmRaw);

  return EFI_SUCCESS;
}

/**
  Read a single random value, in either FIFO or regular mode.

  @param[in]  Val                     A pointer to the 32-bit word that is to
                                      be filled with a random value.

  @retval EFI_SUCCESS                 A random value was successfully read.
  @retval EFI_NOT_READY               The number of retries elapsed before a
                                      random value was generated.

**/
STATIC
EFI_STATUS
EFIAPI
Bcm2836RngReadValue (
  IN OUT  UINT32                  *Val
)
{
  UINT32 Avail;
  UINT32 i;
  BOOLEAN UseFifo = FixedPcdGetBool (PcdBcm283xRngUseFifo);

  ASSERT (Val != NULL);

  Avail = (UseFifo) ?
    (MmioRead32 (RNG_FIFO_COUNT) & RNG_FIFO_NUMVAL_MASK) :
    (MmioRead32 (RNG_STATUS) >> RNG_STATUS_NUMVAL_SHIFT);

  //
  // If we don't have a value ready, wait 1 us and retry.
  //
  for (i = 0; Avail < 1 && i < RNG_MAX_RETRIES; i++) {
    MicroSecondDelay (1);
    Avail = (UseFifo) ?
      (MmioRead32 (RNG_FIFO_COUNT) & RNG_FIFO_NUMVAL_MASK) :
      (MmioRead32 (RNG_STATUS) >> RNG_STATUS_NUMVAL_SHIFT);
  }
  if (Avail < 1) {
    return EFI_NOT_READY;
  }

  *Val = (UseFifo) ?
    MmioRead32 (RNG_FIFO_DATA):
    MmioRead32 (RNG_DATA);

  return EFI_SUCCESS;
}

/**
  Produces and returns an RNG value using either the default or specified RNG
  algorithm.

  @param[in]  This                    A pointer to the EFI_RNG_PROTOCOL
                                      instance.
  @param[in]  RNGAlgorithm            A pointer to the EFI_RNG_ALGORITHM that
                                      identifies the RNG algorithm to use. May
                                      be NULL in which case the function will
                                      use its default RNG algorithm.
  @param[in]  RNGValueLength          The length in bytes of the memory buffer
                                      pointed to by RNGValue. The driver shall
                                      return exactly this numbers of bytes.
  @param[out] RNGValue                A caller-allocated memory buffer filled
                                      by the driver with the resulting RNG
                                      value.

  @retval EFI_SUCCESS                 The RNG value was returned successfully.
  @retval EFI_UNSUPPORTED             The algorithm specified by RNGAlgorithm
                                      is not supported by this driver.
  @retval EFI_DEVICE_ERROR            An RNG value could not be retrieved due
                                      to a hardware or firmware error.
  @retval EFI_NOT_READY               There is not enough random data available
                                      to satisfy the length requested by
                                      RNGValueLength.
  @retval EFI_INVALID_PARAMETER       RNGValue is NULL or RNGValueLength is
                                      zero.

**/
STATIC
EFI_STATUS
EFIAPI
Bcm2836RngGetRNG (
  IN EFI_RNG_PROTOCOL            *This,
  IN EFI_RNG_ALGORITHM           *RNGAlgorithm, OPTIONAL
  IN UINTN                       RNGValueLength,
  OUT UINT8                      *RNGValue
  )
{
  EFI_STATUS      Status;
  UINT32          Val;

  if (This == NULL || RNGValueLength == 0 || RNGValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // We only support the raw algorithm, so reject requests for anything else
  //
  if (RNGAlgorithm != NULL &&
      !CompareGuid (RNGAlgorithm, &gEfiRngAlgorithmRaw)) {
    return EFI_UNSUPPORTED;
  }

  while (RNGValueLength >= sizeof (UINT32)) {
    Status = Bcm2836RngReadValue (&Val);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    WriteUnaligned32 ((VOID *)RNGValue, Val);
    RNGValue += sizeof (UINT32);
    RNGValueLength -= sizeof (UINT32);
  }

  if (RNGValueLength > 0) {
    Status = Bcm2836RngReadValue (&Val);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    while (RNGValueLength--) {
      *RNGValue++ = (UINT8)Val;
      Val >>= 8;
    }
  }
  return EFI_SUCCESS;
}

STATIC EFI_RNG_PROTOCOL mBcm2836RngProtocol = {
  Bcm2836RngGetInfo,
  Bcm2836RngGetRNG
};

//
// Entry point of this driver.
//
EFI_STATUS
EFIAPI
Bcm2836RngEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS      Status;

  Status = gBS->InstallMultipleProtocolInterfaces (&ImageHandle,
                  &gEfiRngProtocolGuid, &mBcm2836RngProtocol,
                  NULL);
  ASSERT_EFI_ERROR (Status);

  MmioWrite32 (RNG_BIT_COUNT_THRESHOLD, RNG_WARMUP_COUNT);
  MmioWrite32 (RNG_CTRL, RNG_CTRL_ENABLE);

  return EFI_SUCCESS;
}
