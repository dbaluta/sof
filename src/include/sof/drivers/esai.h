/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019 NXP
 *
 * Author: Daniel Baluta <daniel.baluta@nxp.com>
 */

#ifndef __SOF_DRIVERS_ESAI_H__
#define __SOF_DRIVERS_ESAI_H__

#include <sof/lib/dai.h>

/* ESAI Register Map */
#define REG_ESAI_ETDR           0x00
#define REG_ESAI_ERDR           0x04
#define REG_ESAI_ECR            0x08
#define REG_ESAI_ESR            0x0C
#define REG_ESAI_TFCR           0x10
#define REG_ESAI_TFSR           0x14
#define REG_ESAI_RFCR           0x18
#define REG_ESAI_RFSR           0x1C
#define REG_ESAI_xFCR(tx)       (tx ? REG_ESAI_TFCR : REG_ESAI_RFCR)
#define REG_ESAI_xFSR(tx)       (tx ? REG_ESAI_TFSR : REG_ESAI_RFSR)
#define REG_ESAI_TX0            0x80
#define REG_ESAI_TX1            0x84
#define REG_ESAI_TX2            0x88
#define REG_ESAI_TX3            0x8C
#define REG_ESAI_TX4            0x90
#define REG_ESAI_TX5            0x94
#define REG_ESAI_TSR            0x98
#define REG_ESAI_RX0            0xA0
#define REG_ESAI_RX1            0xA4
#define REG_ESAI_RX2            0xA8
#define REG_ESAI_RX3            0xAC
#define REG_ESAI_SAISR          0xCC
#define REG_ESAI_SAICR          0xD0
#define REG_ESAI_TCR            0xD4
#define REG_ESAI_TCCR           0xD8
#define REG_ESAI_RCR            0xDC
#define REG_ESAI_RCCR           0xE0
#define REG_ESAI_xCR(tx)        (tx ? REG_ESAI_TCR : REG_ESAI_RCR)
#define REG_ESAI_xCCR(tx)       (tx ? REG_ESAI_TCCR : REG_ESAI_RCCR)
#define REG_ESAI_TSMA           0xE4
#define REG_ESAI_TSMB           0xE8
#define REG_ESAI_RSMA           0xEC
#define REG_ESAI_RSMB           0xF0
#define REG_ESAI_xSMA(tx)       (tx ? REG_ESAI_TSMA : REG_ESAI_RSMA)
#define REG_ESAI_xSMB(tx)       (tx ? REG_ESAI_TSMB : REG_ESAI_RSMB)
#define REG_ESAI_PRRC           0xF8
#define REG_ESAI_PCRC           0xFC

/* ESAI Control Register (ECR) */
#define ESAI_ECR_ETI	BIT(19)
#define ESAI_ECR_ETO	BIT(18)
#define ESAI_ECR_ERI	BIT(17)
#define ESAI_ECR_ERO	BIT(16)
#define ESAI_ECR_ERST	BIT(1)
#define ESAI_ECR_ESAIEN	BIT(0)

/* ESAI Status Register (ESR) */
#define ESAI_ESR_TINIT	BIT(10)
#define ESAI_ESR_RFF	BIT(9)
#define ESAI_ESR_TFE	BIT(8)
#define ESAI_ESR_TLS	BIT(7)
#define ESAI_ESR_TDE	BIT(6)
#define ESAI_ESR_TED	BIT(5)
#define ESAI_ESR_TD	BIT(4)
#define ESAI_ESR_RLS	BIT(3)
#define ESAI_ESR_RDE	BIT(2)
#define ESAI_ESR_RED	BIT(1)
#define ESAI_ESR_RD	BIT(0)

/* Tx/Rx FIFO Configuration Register (xFCR) */
#define ESAI_xFCR_TIEN	BIT(19)
#define ESAI_xFCR_REXT	BIT(19)
#define ESAI_xFCR_xWA_SHIFT     16
#define ESAI_xFCR_xWA_WIDTH     3
#define ESAI_xFCR_xWA_MASK      (((1 << ESAI_xFCR_xWA_WIDTH) - 1) << ESAI_xFCR_xWA_SHIFT)
#define ESAI_xFCR_xWA(v)        (((8 - ((v) >> 2)) << ESAI_xFCR_xWA_SHIFT) & ESAI_xFCR_xWA_MASK)
#define ESAI_xFCR_xFWM_SHIFT    8
#define ESAI_xFCR_xFWM_WIDTH    8
#define ESAI_xFCR_xFWM_MASK     (((1 << ESAI_xFCR_xFWM_WIDTH) - 1) << ESAI_xFCR_xFWM_SHIFT)
#define ESAI_xFCR_xFWM(v)       ((((v) - 1) << ESAI_xFCR_xFWM_SHIFT) & ESAI_xFCR_xFWM_MASK)
#define ESAI_xFCR_xE_SHIFT      2
#define ESAI_xFCR_TE_WIDTH      6
#define ESAI_xFCR_RE_WIDTH      4
#define ESAI_xFCR_TE_MASK       (((1 << ESAI_xFCR_TE_WIDTH) - 1) << ESAI_xFCR_xE_SHIFT)
#define ESAI_xFCR_RE_MASK       (((1 << ESAI_xFCR_RE_WIDTH) - 1) << ESAI_xFCR_xE_SHIFT)
#define ESAI_xFCR_TE(x)         ((ESAI_xFCR_TE_MASK >> (ESAI_xFCR_TE_WIDTH - x)) & ESAI_xFCR_TE_MASK)
#define ESAI_xFCR_RE(x)         ((ESAI_xFCR_RE_MASK >> (ESAI_xFCR_RE_WIDTH - x)) & ESAI_xFCR_RE_MASK)
#define ESAI_xFCR_xFR	BIT(1)
#define ESAI_xFCR_xFEN	BIT(0)

/* Tx/Rx FIFO Status Register (REG_ESAI_xFSR) */
#define ESAI_xFSR_NTFO_MASK	MASK(14, 12)
#define ESAI_xFSR_NTFI_MASK	MASK(10, 8)
#define ESAI_xFSR_NRFO_MASK	MASK(9, 8)
#define ESAI_xFSR_NRFI_MASK	MASK(13, 12)
#define ESAI_xFSR_xFCNT_MASK	MASK(7, 0)

/* ESAI Transmit Slot Register (REG_ESAI_TSR)*/
#define ESAI_TSR_MASK	MASK(23, 0)

/* Serial Audio Interface Status Register (REG_ESAI_SAISR) */
#define ESAI_SAISR_TODFE	BIT(17)
#define ESAI_SAISR_TEDE		BIT(16)
#define ESAI_SAISR_TDE		BIT(15)
#define ESAI_SAISR_TUE		BIT(14)
#define ESAI_SAISR_TFS		BIT(13)
#define ESAI_SAISR_RODF		BIT(10)
#define ESAI_SAISR_REDF		BIT(9)
#define ESAI_SAISR_RDF		BIT(8)
#define ESAI_SAISR_ROE		BIT(7)
#define ESAI_SAISR_RFS		BIT(6)
#define ESAI_SAISR_IF2		BIT(2)
#define ESAI_SAISR_IF1		BIT(1)
#define ESAI_SAISR_IF0		BIT(0)

/* Serial Audio Interface Control Register -- REG_ESAI_SAICR 0xD0 */
#define ESAI_SAICR_ALC		BIT(8)
#define ESAI_SAICR_TEBE		BIT(7)
#define ESAI_SAICR_SYNC		BIT(6)
#define ESAI_SAICR_OF2		BIT(2)
#define ESAI_SAICR_OF1		BIT(1)
#define ESAI_SAICR_OF0		BIT(0)

/* Tx/Rx Control Register (REG_ESAI_xCR) */
#define ESAI_xCR_xLIE		BIT(23)
#define ESAI_xCR_xIE		BIT(22)
#define ESAI_xCR_xEDIE		BIT(21)
#define ESAI_xCR_xEIE		BIT(20)
#define ESAI_xCR_xPR		BIT(19)
#define ESAI_xCR_PADC		BIT(17)
#define ESAI_xCR_xFSR		BIT(16)
#define ESAI_xCR_xFSL		BIT(15)
#define ESAI_xCR_xSWS_SHIFT     10
#define ESAI_xCR_xSWS_WIDTH     5
#define ESAI_xCR_xSWS_MASK      (((1 << ESAI_xCR_xSWS_WIDTH) - 1) << ESAI_xCR_xSWS_SHIFT)
#define ESAI_xCR_xSWS(s, w)     ((w < 24 ? (s - w + ((w - 8) >> 2)) : (s < 32 ? 0x1e : 0x1f)) << ESAI_xCR_xSWS_SHIFT)
#define ESAI_xCR_xMOD_SHIFT     8
#define ESAI_xCR_xMOD_WIDTH     2
#define ESAI_xCR_xMOD_MASK      (((1 << ESAI_xCR_xMOD_WIDTH) - 1) << ESAI_xCR_xMOD_SHIFT)
#define ESAI_xCR_xMOD_ONDEMAND  (0x1 << ESAI_xCR_xMOD_SHIFT)
#define ESAI_xCR_xMOD_NETWORK   (0x1 << ESAI_xCR_xMOD_SHIFT)
#define ESAI_xCR_xMOD_AC97      (0x3 << ESAI_xCR_xMOD_SHIFT)
#define ESAI_xCR_xWA		BIT(7)
#define ESAI_xCR_xSHFD		BIT(6)
#define ESAI_xCR_xE_SHIFT       0
#define ESAI_xCR_TE_WIDTH       6
#define ESAI_xCR_RE_WIDTH       4
#define ESAI_xCR_TE_MASK        (((1 << ESAI_xCR_TE_WIDTH) - 1) << ESAI_xCR_xE_SHIFT)
#define ESAI_xCR_RE_MASK        (((1 << ESAI_xCR_RE_WIDTH) - 1) << ESAI_xCR_xE_SHIFT)
#define ESAI_xCR_TE(x)          ((ESAI_xCR_TE_MASK >> (ESAI_xCR_TE_WIDTH - x)) & ESAI_xCR_TE_MASK)
#define ESAI_xCR_RE(x)          ((ESAI_xCR_RE_MASK >> (ESAI_xCR_RE_WIDTH - x)) & ESAI_xCR_RE_MASK)

/* Tx/Rx Clock Control Register (REG_ESAI_xCCR) */
#define ESAI_xCCR_xHCKD		BIT(23)
#define ESAI_xCCR_xFSD		BIT(22)
#define ESAI_xCCR_xCKD		BIT(21)
#define ESAI_xCCR_xHCKP		BIT(20)
#define ESAI_xCCR_xFSP		BIT(19)
#define ESAI_xCCR_xCKP		BIT(18)
#define ESAI_xCCR_xFP_MASK	MASK(17, 14)
#define ESAI_xCCR_xFP(v)	SET_BITS(17, 14, (v))
#define ESAI_xCCR_xDC_MASK	MASK(13, 9)_
#define ESAI_xCCR_xDC(v)	SET_BITS(13, 9, (v))
#define ESAI_xCCR_xPSR_MASK	BIT(8)
#define ESAI_xCCR_xPSR_BYPASS	SET_BITS(8, 8, 1)
#define ESAI_xCCR_xPSR_DIV8     SET_BITS(8, 8, 0)
#define ESAI_xCCR_xPM_MASK	MASK(7, 0)
#define ESAI_xCCR_xPM(v)	SET_BITS(7, 0, (v))

/* Transmit Slot Mask Register A/B (REG_ESAI_TSMA/B) */
#define ESAI_xSMA_xS_MASK	MASK(15, 0)
#define ESAI_xSMA_xS(v)		SET_BITS(15, 0, (v))
#define ESAI_xSMB_xS_MASK	MASK(15, 0)
#define ESAI_xSMB_xS(v)		SET_BITS(15, 0, (v))

/* Port C Direction Register (REG_ESAI_PRRC)*/
#define ESAI_PRRC_PDC_MASK	MASK(11, 0)
#define ESAI_PRRC_PDC(v)	SET_BITS(11, 0, (v))

/* Port C Control Register (REG_ESAI_PCRC) */
#define ESAI_PCRC_PC_SHIFT      0
#define ESAI_PCRC_PC_WIDTH      12
#define ESAI_PCRC_PC_MASK	MASK(11, 0)
#define ESAI_PCRC_PC(v)		SET_BITS(11, 0, (v))

extern const struct dai_driver esai_driver;
#endif /* __SOF_DRIVERS_ESAI_H__ */
