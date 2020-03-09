/** @file
  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "GenericPhy.h"

#define PHY_RESET_TIMEOUT       500
#define PHY_LINK_TIMEOUT        200
#define PHY_ANEG_TIMEOUT        200

STATIC
EFI_STATUS
GenericPhyRead (
    IN GENERIC_PHY_PRIVATE_DATA *Phy,
    IN UINT8                    PhyAddr,
    IN UINT8                    Reg,
    OUT UINT16 *                Data
    )
{
    return Phy->Read (Phy->PrivateData, PhyAddr, Reg, Data);
}

STATIC
EFI_STATUS
GenericPhyWrite (
    IN GENERIC_PHY_PRIVATE_DATA *Phy,
    IN UINT8                    PhyAddr,
    IN UINT8                    Reg,
    IN UINT16                   Data
    )
{
    return Phy->Write (Phy->PrivateData, PhyAddr, Reg, Data);
}

STATIC
VOID
GenericPhyConfigure (
    IN GENERIC_PHY_PRIVATE_DATA *Phy,
    IN GENERIC_PHY_SPEED        Speed,
    IN GENERIC_PHY_DUPLEX       Duplex
    )
{
    Phy->Configure (Phy->PrivateData, Speed, Duplex);
}

STATIC
EFI_STATUS
GenericPhyDetect (
    IN GENERIC_PHY_PRIVATE_DATA *Phy
    )
{
    EFI_STATUS Status;
    UINT8 PhyAddr;
    UINT16 Id1, Id2;

    for (PhyAddr = 0; PhyAddr < 32; PhyAddr++) {
        Status = GenericPhyRead (Phy, PhyAddr, GENERIC_PHY_PHYIDR1, &Id1);
        if (EFI_ERROR (Status)) {
            continue;
        }
        Status = GenericPhyRead (Phy, PhyAddr, GENERIC_PHY_PHYIDR2, &Id2);
        if (EFI_ERROR (Status)) {
            continue;
        }
        if (Id1 != 0xFFFF && Id2 != 0xFFFF) {
            Phy->PhyAddr = PhyAddr;
            DEBUG ((EFI_D_INFO, "GenericPhyDetect: PHY detected at address 0x%02X (PHYIDR1=0x%04X, PHYIDR2=0x%04X)\n",
                    PhyAddr, Id1, Id2));
            return EFI_SUCCESS;
        }
    }

    return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
GenericPhyAutoNegotiate (
    IN GENERIC_PHY_PRIVATE_DATA *Phy
    )
{
    EFI_STATUS Status;
    UINT16 Anar, Gbcr, Bmcr;

    Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_ANAR, &Anar);
    if (EFI_ERROR (Status)) {
        return Status;
    }
    Anar |= GENERIC_PHY_ANAR_100BASETX_FDX |
            GENERIC_PHY_ANAR_100BASETX |
            GENERIC_PHY_ANAR_10BASET_FDX |
            GENERIC_PHY_ANAR_10BASET;
    Status = GenericPhyWrite (Phy, Phy->PhyAddr, GENERIC_PHY_ANAR, Anar);
    if (EFI_ERROR (Status)) {
        return Status;
    }

    Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_GBCR, &Gbcr);
    if (EFI_ERROR (Status)) {
        return Status;
    }
    Gbcr |= GENERIC_PHY_GBCR_1000BASET_FDX |
            GENERIC_PHY_GBCR_1000BASET;
    Status = GenericPhyWrite (Phy, Phy->PhyAddr, GENERIC_PHY_GBCR, Gbcr);
    if (EFI_ERROR (Status)) {
        return Status;
    }

    Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_BMCR, &Bmcr);
    if (EFI_ERROR (Status)) {
        return Status;
    }
    Bmcr |= GENERIC_PHY_BMCR_ANE |
            GENERIC_PHY_BMCR_RESTART_AN;
    return GenericPhyWrite (Phy, Phy->PhyAddr, GENERIC_PHY_BMCR, Bmcr);
}

EFI_STATUS
EFIAPI
GenericPhyInit (
    IN GENERIC_PHY_PRIVATE_DATA *Phy
    )
{
    EFI_STATUS Status;

    ASSERT (Phy->Read != NULL);
    ASSERT (Phy->Write != NULL);
    ASSERT (Phy->Configure != NULL);

    Status = GenericPhyDetect (Phy);
    if (EFI_ERROR (Status)) {
        return Status;
    }

    Status = GenericPhyReset (Phy);
    if (EFI_ERROR (Status)) {
        return Status;
    }

    return GenericPhyAutoNegotiate (Phy);
}

EFI_STATUS
EFIAPI
GenericPhyReset (
    IN GENERIC_PHY_PRIVATE_DATA *Phy
    )
{
    EFI_STATUS Status;
    UINTN Retry;
    UINT16 Data;

    // Start reset sequence
    Status = GenericPhyWrite (Phy, Phy->PhyAddr, GENERIC_PHY_BMCR, GENERIC_PHY_BMCR_RESET);
    if (EFI_ERROR (Status)) {
        return Status;
    }

    // Wait up to 500ms for it to complete
    for (Retry = PHY_RESET_TIMEOUT; Retry > 0; Retry--) {
        Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_BMCR, &Data);
        if (EFI_ERROR (Status)) {
            return Status;
        }
        if ((Data & GENERIC_PHY_BMCR_RESET) == 0) {
            break;
        }
        gBS->Stall (1000);
    }
    if (Retry == 0) {
        return EFI_TIMEOUT;
    }

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GenericPhyGetLinkStatus (
    IN GENERIC_PHY_PRIVATE_DATA *Phy
    )
{
    EFI_STATUS Status;
    UINT16 Bmsr;
    UINTN Retry;

    Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_BMSR, &Bmsr);
    if (EFI_ERROR (Status)) {
        return Status;
    }

    if ((Bmsr & GENERIC_PHY_BMSR_LINK_STATUS) != 0) {
        // Link is up
        return EFI_SUCCESS;
    }

    // Wait for link to come up
    for (Retry = PHY_LINK_TIMEOUT; Retry > 0; Retry--) {
        Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_BMSR, &Bmsr);
        if (EFI_ERROR (Status)) {
            return Status;
        }

        if ((Bmsr & GENERIC_PHY_BMSR_LINK_STATUS) != 0) {
            // Link is up
            break;
        }

        gBS->Stall (1000);
    }
    if (Retry == 0) {
        // Link is down
        return EFI_TIMEOUT;
    }

    // Wait for auto negotiate to complete
    for (Retry = PHY_ANEG_TIMEOUT; Retry > 0; Retry--) {
        Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_BMSR, &Bmsr);
        if (EFI_ERROR (Status)) {
            return Status;
        }

        if ((Bmsr & GENERIC_PHY_BMSR_ANEG_COMPLETE) != 0) {
            // Auto negotiation complete
            break;
        }

        gBS->Stall (1000);
    }
    if (Retry == 0) {
        DEBUG ((EFI_D_ERROR, "GenericPhyGetLinkStatus: Auto-negotiation timeout (BMSR=0x%04X)\n", Bmsr));
        return EFI_TIMEOUT;
    }

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GenericPhyGetConfig (
    IN GENERIC_PHY_PRIVATE_DATA *Phy,
    OUT GENERIC_PHY_SPEED       *Speed,
    OUT GENERIC_PHY_DUPLEX      *Duplex
    )
{
    EFI_STATUS Status;
    UINT16 Gbcr, Gbsr, Anlpar, Anar;
    UINT16 Gb, An;

    Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_GBCR, &Gbcr);
    if (EFI_ERROR (Status)) {
        return Status;
    }
    Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_GBSR, &Gbsr);
    if (EFI_ERROR (Status)) {
        return Status;
    }
    Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_ANLPAR, &Anlpar);
    if (EFI_ERROR (Status)) {
        return Status;
    }
    Status = GenericPhyRead (Phy, Phy->PhyAddr, GENERIC_PHY_ANAR, &Anar);
    if (EFI_ERROR (Status)) {
        return Status;
    }

    Gb = (Gbsr >> 2) & Gbcr;
    An = Anlpar & Anar;

    //DEBUG ((EFI_D_INFO, "GenericPhyGetConfig: Gtsr=0x%04X Gtcr=0x%04X Gt=0x%04X\n", Gtsr, Gtcr, Gt));
    //DEBUG ((EFI_D_INFO, "GenericPhyGetConfig: Anlpar=0x%04X Anar=0x%04X An=0x%04X\n", Anlpar, Anar, An));

    if ((Gb & (GENERIC_PHY_GBCR_1000BASET_FDX|GENERIC_PHY_GBCR_1000BASET)) != 0) {
        *Speed = PHY_SPEED_1000;
        *Duplex = (Gb & GENERIC_PHY_GBCR_1000BASET_FDX) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
    } else if ((An & (GENERIC_PHY_ANAR_100BASETX_FDX|GENERIC_PHY_ANAR_100BASETX)) != 0) {
        *Speed = PHY_SPEED_100;
        *Duplex = (An & GENERIC_PHY_ANAR_100BASETX_FDX) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
    } else {
        *Speed = PHY_SPEED_10;
        *Duplex = (An & GENERIC_PHY_ANAR_10BASET_FDX) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
    }

    DEBUG ((EFI_D_INFO, "GenericPhyGetConfig: Link speed %d Mbps, %a-duplex\n",
           *Speed, *Duplex == PHY_DUPLEX_FULL ? "full" : "half"));

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GenericPhyUpdateConfig (
    IN GENERIC_PHY_PRIVATE_DATA *Phy
    )
{
    EFI_STATUS Status;
    GENERIC_PHY_SPEED Speed;
    GENERIC_PHY_DUPLEX Duplex;
    BOOLEAN LinkUp;

    Status = GenericPhyGetLinkStatus (Phy);
    LinkUp = EFI_ERROR (Status) ? FALSE : TRUE;

    if (Phy->LinkUp != LinkUp) {
        if (LinkUp) {
            DEBUG ((EFI_D_INFO, "GenericPhyUpdateConfig: Link is up\n"));

            Status = GenericPhyGetConfig (Phy, &Speed, &Duplex);
            if (EFI_ERROR (Status)) {
                return Status;
            }

            GenericPhyConfigure (Phy, Speed, Duplex);
        } else {
            DEBUG ((EFI_D_INFO, "GenericPhyUpdateConfig: Link is down\n"));
        }
    }

    Phy->LinkUp = LinkUp;

    return LinkUp ? EFI_SUCCESS : EFI_NOT_READY;
}