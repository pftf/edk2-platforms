/** @file
  Copyright (c) 2020 Jared McNeill. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause
**/

#ifndef GENETREG_H__
#define GENETREG_H__

#define	GENET_SYS_REV_CTRL		                0x000
#define	 SYS_REV_MAJOR			                (BIT27|BIT26|BIT25|BIT24)
#define	 SYS_REV_MINOR			                (BIT19|BIT18|BIT17|BIT16)
#define	GENET_SYS_PORT_CTRL		                0x004
#define	 GENET_SYS_PORT_MODE_EXT_GPHY	        3
#define	GENET_SYS_RBUF_FLUSH_CTRL	            0x008
#define	 GENET_SYS_RBUF_FLUSH_RESET	            BIT1
#define	GENET_SYS_TBUF_FLUSH_CTRL	            0x00c
#define	GENET_EXT_RGMII_OOB_CTRL	            0x08c
#define	 GENET_EXT_RGMII_OOB_ID_MODE_DISABLE	BIT16
#define	 GENET_EXT_RGMII_OOB_RGMII_MODE_EN	    BIT6
#define	 GENET_EXT_RGMII_OOB_OOB_DISABLE	    BIT5
#define	 GENET_EXT_RGMII_OOB_RGMII_LINK		    BIT4
#define	GENET_INTRL2_CPU_STAT		            0x200
#define	GENET_INTRL2_CPU_CLEAR		            0x208
#define	GENET_INTRL2_CPU_STAT_MASK	            0x20c
#define	GENET_INTRL2_CPU_SET_MASK	            0x210
#define	GENET_INTRL2_CPU_CLEAR_MASK	            0x214
#define	 GENET_IRQ_MDIO_ERROR		            BIT24
#define	 GENET_IRQ_MDIO_DONE		            BIT23
#define	 GENET_IRQ_TXDMA_DONE		            BIT16
#define	 GENET_IRQ_RXDMA_DONE		            BIT13
#define	GENET_RBUF_CTRL			                0x300
#define	 GENET_RBUF_BAD_DIS		                BIT2
#define	 GENET_RBUF_ALIGN_2B		            BIT1
#define	 GENET_RBUF_64B_EN		                BIT0
#define	GENET_RBUF_TBUF_SIZE_CTRL	            0x3b4
#define	GENET_UMAC_CMD			                0x808
#define	 GENET_UMAC_CMD_LCL_LOOP_EN	            BIT15
#define	 GENET_UMAC_CMD_SW_RESET	            BIT13
#define	 GENET_UMAC_CMD_PROMISC		            BIT4
#define	 GENET_UMAC_CMD_SPEED		            (BIT3|BIT2)
#define	  GENET_UMAC_CMD_SPEED_10	            0
#define	  GENET_UMAC_CMD_SPEED_100	            1
#define	  GENET_UMAC_CMD_SPEED_1000	            2
#define	 GENET_UMAC_CMD_RXEN		            BIT1
#define	 GENET_UMAC_CMD_TXEN		            BIT0
#define	GENET_UMAC_MAC0			                0x80c
#define	GENET_UMAC_MAC1			                0x810
#define	GENET_UMAC_MAX_FRAME_LEN	            0x814
#define	GENET_UMAC_TX_FLUSH		                0xb34
#define	GENET_UMAC_MIB_CTRL		                0xd80
#define	 GENET_UMAC_MIB_RESET_TX	            BIT2
#define	 GENET_UMAC_MIB_RESET_RUNT	            BIT1
#define	 GENET_UMAC_MIB_RESET_RX	            BIT0
#define	GENET_MDIO_CMD			                0xe14
#define	 GENET_MDIO_START_BUSY		            BIT29
#define	 GENET_MDIO_READ		                BIT27
#define	 GENET_MDIO_WRITE		                BIT26
#define	 GENET_MDIO_PMD			                (BIT25|BIT24|BIT23|BIT22|BIT21)
#define	 GENET_MDIO_REG			                (BIT20|BIT19|BIT18|BIT17|BIT16)
#define	GENET_UMAC_MDF_CTRL		                0xe50
#define	GENET_UMAC_MDF_ADDR0(n)		            (0xe54 + (n) * 0x8)
#define	GENET_UMAC_MDF_ADDR1(n)		            (0xe58 + (n) * 0x8)

#define	GENET_DMA_DESC_COUNT		            256
#define	GENET_DMA_DESC_SIZE		                12
#define	GENET_DMA_DEFAULT_QUEUE		            16

#define	GENET_DMA_RING_SIZE		                0x40
#define	GENET_DMA_RINGS_SIZE		            (GENET_DMA_RING_SIZE * (GENET_DMA_DEFAULT_QUEUE + 1))

#define	GENET_RX_BASE			                0x2000
#define	GENET_TX_BASE			                0x4000

#define	GENET_RX_DMA_RINGBASE(qid)	            (GENET_RX_BASE + 0xc00 + GENET_DMA_RING_SIZE * (qid))
#define	GENET_RX_DMA_WRITE_PTR_LO(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x00)
#define	GENET_RX_DMA_WRITE_PTR_HI(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x04)
#define	GENET_RX_DMA_PROD_INDEX(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x08)
#define	GENET_RX_DMA_CONS_INDEX(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x0c)
#define	GENET_RX_DMA_RING_BUF_SIZE(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x10)
#define	 GENET_RX_DMA_RING_BUF_SIZE_DESC_COUNT	0xffff0000
#define	 GENET_RX_DMA_RING_BUF_SIZE_BUF_LENGTH	0x0000ffff
#define	GENET_RX_DMA_START_ADDR_LO(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x14)
#define	GENET_RX_DMA_START_ADDR_HI(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x18)
#define	GENET_RX_DMA_END_ADDR_LO(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x1c)
#define	GENET_RX_DMA_END_ADDR_HI(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x20)
#define	GENET_RX_DMA_XON_XOFF_THRES(qid)        (GENET_RX_DMA_RINGBASE(qid) + 0x28)
#define	 GENET_RX_DMA_XON_XOFF_THRES_LO		    0xffff0000
#define	 GENET_RX_DMA_XON_XOFF_THRES_HI		    0x0000ffff
#define	GENET_RX_DMA_READ_PTR_LO(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x2c)
#define	GENET_RX_DMA_READ_PTR_HI(qid)	        (GENET_RX_DMA_RINGBASE(qid) + 0x30)

#define	GENET_TX_DMA_RINGBASE(qid)	            (GENET_TX_BASE + 0xc00 + GENET_DMA_RING_SIZE * (qid))
#define	GENET_TX_DMA_READ_PTR_LO(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x00)
#define	GENET_TX_DMA_READ_PTR_HI(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x04)
#define	GENET_TX_DMA_CONS_INDEX(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x08)
#define	GENET_TX_DMA_PROD_INDEX(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x0c)
#define	GENET_TX_DMA_RING_BUF_SIZE(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x10)
#define	 GENET_TX_DMA_RING_BUF_SIZE_DESC_COUNT	0xffff0000
#define	 GENET_TX_DMA_RING_BUF_SIZE_BUF_LENGTH	0x0000ffff
#define	GENET_TX_DMA_START_ADDR_LO(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x14)
#define	GENET_TX_DMA_START_ADDR_HI(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x18)
#define	GENET_TX_DMA_END_ADDR_LO(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x1c)
#define	GENET_TX_DMA_END_ADDR_HI(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x20)
#define	GENET_TX_DMA_MBUF_DONE_THRES(qid)       (GENET_TX_DMA_RINGBASE(qid) + 0x24)
#define	GENET_TX_DMA_FLOW_PERIOD(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x28)
#define	GENET_TX_DMA_WRITE_PTR_LO(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x2c)
#define	GENET_TX_DMA_WRITE_PTR_HI(qid)	        (GENET_TX_DMA_RINGBASE(qid) + 0x30)

#define	GENET_RX_DESC_STATUS(idx)	            (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x00)
#define	 GENET_RX_DESC_STATUS_BUFLEN	        (BIT27|BIT26|BIT25|BIT24|BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
#define	 GENET_RX_DESC_STATUS_OWN	            BIT15
#define	 GENET_RX_DESC_STATUS_EOP	            BIT14
#define	 GENET_RX_DESC_STATUS_SOP	            BIT13
#define	 GENET_RX_DESC_STATUS_RX_ERROR	        BIT2
#define	GENET_RX_DESC_ADDRESS_LO(idx)	        (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x04)
#define	GENET_RX_DESC_ADDRESS_HI(idx)	        (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x08)

#define	GENET_TX_DESC_STATUS(idx)	            (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x00)
#define	 GENET_TX_DESC_STATUS_BUFLEN	        (BIT27|BIT26|BIT25|BIT24|BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
#define	 GENET_TX_DESC_STATUS_OWN	            BIT15
#define	 GENET_TX_DESC_STATUS_EOP	            BIT14
#define	 GENET_TX_DESC_STATUS_SOP	            BIT13
#define	 GENET_TX_DESC_STATUS_QTAG	            (BIT12|BIT11|BIT10|BIT9|BIT8|BIT7)
#define	 GENET_TX_DESC_STATUS_CRC	            BIT6
#define	GENET_TX_DESC_ADDRESS_LO(idx)	        (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x04)
#define	GENET_TX_DESC_ADDRESS_HI(idx)	        (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x08)

#define	GENET_RX_DMA_RING_CFG		            (GENET_RX_BASE + 0x1040 + 0x00)
#define	GENET_RX_DMA_CTRL		                (GENET_RX_BASE + 0x1040 + 0x04)
#define	 GENET_RX_DMA_CTRL_RBUF_EN(qid)	        (BIT1 << (qid))
#define	 GENET_RX_DMA_CTRL_EN		            BIT0
#define	GENET_RX_SCB_BURST_SIZE		            (GENET_RX_BASE + 0x1040 + 0x0c)

#define	GENET_TX_DMA_RING_CFG		            (GENET_TX_BASE + 0x1040 + 0x00)
#define	GENET_TX_DMA_CTRL		                (GENET_TX_BASE + 0x1040 + 0x04)
#define	 GENET_TX_DMA_CTRL_RBUF_EN(qid)	        (BIT1 << (qid))
#define	 GENET_TX_DMA_CTRL_EN		            BIT0
#define	GENET_TX_SCB_BURST_SIZE		            (GENET_TX_BASE + 0x1040 + 0x0c)

#endif // GENETREG_H__