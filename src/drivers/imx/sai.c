// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright 2019 NXP
//
// Author: Daniel Baluta <daniel.baluta@nxp.com>
// Author: Jerome Laclavere <jerome.laclavere@nxp.com>
// Author: Guido Roncarolo <guido.roncarolo@nxp.com>

#include <sof/audio/component.h>
#include <sof/bit.h>
#include <sof/drivers/edma.h>
#include <sof/drivers/sai.h>
#include <sof/lib/dai.h>
#include <sof/lib/dma.h>
#include <sof/lib/uuid.h>
#include <ipc/dai.h>
#include <errno.h>
#include <stdint.h>

/* 9302adf5-88be-4234-a0a7-dca538ef81f4 */
DECLARE_SOF_UUID("sai", sai_uuid, 0x9302adf5, 0x88be, 0x4234,
		 0xa0, 0xa7, 0xdc, 0xa5, 0x38, 0xef, 0x81, 0xf4);

DECLARE_TR_CTX(sai_tr, SOF_UUID(sai_uuid), LOG_LEVEL_INFO);

#define REG_TX_DIR 0
#define REG_RX_DIR 1

#define FSL_SAI_CSR_SEIE        BIT(11)
#define FSL_SAI_CSR_FEIE        BIT(10)

#define FSL_SAI_CSR_xF_SHIFT    16
#define FSL_SAI_CSR_xF_W_SHIFT  18
#define FSL_SAI_CSR_xF_MASK     (0x1f << FSL_SAI_CSR_xF_SHIFT)
#define FSL_SAI_CSR_xF_W_MASK   (0x7 << FSL_SAI_CSR_xF_W_SHIFT)

#define FSL_SAI_CSR_xIE_SHIFT   8
#define FSL_SAI_CSR_xIE_MASK    (0x1f << FSL_SAI_CSR_xIE_SHIFT)

#define FSL_SAI_FLAGS  (FSL_SAI_CSR_SEIE | FSL_SAI_CSR_FEIE)


static void sai_start(struct dai *dai, int direction)
{
	dai_info(dai, "SAI: sai_start");

	uint32_t xcsr = 0U;

	/* enable DMA requests */
	dai_update_bits(dai, REG_SAI_XCSR(direction),
			REG_SAI_CSR_FRDE, REG_SAI_CSR_FRDE);
#ifdef CONFIG_IMX8M
	dai_update_bits(dai, REG_SAI_MCTL, REG_SAI_MCTL_MCLK_EN,
			REG_SAI_MCTL_MCLK_EN);
#endif

	/* add one word to FIFO before TRCE is enabled */
	if (direction == DAI_DIR_PLAYBACK)
		dai_write(dai, REG_SAI_TDR0, 0x0);
	else
		dai_write(dai, REG_SAI_RDR0, 0x0);

	/* transmitter enable */
	dai_update_bits(dai, REG_SAI_XCSR(direction),
			REG_SAI_CSR_TERE, REG_SAI_CSR_TERE);


	dai_update_bits(dai, REG_SAI_XCSR(direction),
			REG_SAI_CSR_SE, REG_SAI_CSR_SE);

	dai_update_bits(dai, REG_SAI_XCSR(direction),
		FSL_SAI_CSR_xIE_MASK, FSL_SAI_FLAGS);

	/* TODO: for the time being use half FIFO size as watermark */
	dai_update_bits(dai, REG_SAI_XCR1(direction),
			REG_SAI_CR1_RFW_MASK, SAI_FIFO_WORD_SIZE / 2);
	dai_update_bits(dai, REG_SAI_XCR3(direction),
			REG_SAI_CR3_TRCE_MASK, REG_SAI_CR3_TRCE(1));

	if (direction == DAI_DIR_CAPTURE) {
		xcsr = dai_read(dai, REG_SAI_XCSR(DAI_DIR_PLAYBACK));
		if (!(xcsr & REG_SAI_CSR_FRDE)) {

			dai_update_bits(dai, REG_SAI_XCSR(DAI_DIR_PLAYBACK),
					REG_SAI_CSR_TERE, REG_SAI_CSR_TERE);
			dai_update_bits(dai, REG_SAI_XCR3(DAI_DIR_PLAYBACK),
					REG_SAI_CR3_TRCE_MASK,
					REG_SAI_CR3_TRCE(1));
		}
	}

}

static void sai_stop(struct dai *dai, int direction)
{
	static int count = 0;
	dai_err(dai, "SAI: sai_stop");

	uint32_t xcsr = 0U;

	dai_update_bits(dai, REG_SAI_XCSR(direction),
			REG_SAI_CSR_FRDE, 0);
	dai_update_bits(dai, REG_SAI_XCSR(direction),
			REG_SAI_CSR_XIE_MASK, 0);
	dai_err(dai, "sai_stop(%d) .. xcsr %08x", count++, dai_read(dai, REG_SAI_XCSR(direction)));
	/* Check if the opposite direction is also disabled */
	xcsr = dai_read(dai, REG_SAI_XCSR(!direction));
	if (!(xcsr & REG_SAI_CSR_FRDE)) {
		/* Disable both directions and reset their FIFOs */
		dai_update_bits(dai, REG_SAI_TCSR, REG_SAI_CSR_TERE, 0);
		dai_update_bits(dai, REG_SAI_RCSR, REG_SAI_CSR_TERE, 0);

		do {
			xcsr = dai_read(dai, REG_SAI_XCSR(direction));
		} while (xcsr & REG_SAI_CSR_TERE);

		dai_err(dai, "SAI: sai_stop OKK!");
	
		dai_update_bits(dai, REG_SAI_TCSR, REG_SAI_CSR_FR, REG_SAI_CSR_FR);
		dai_update_bits(dai, REG_SAI_RCSR, REG_SAI_CSR_FR, REG_SAI_CSR_FR);

		/* Software Reset for both Tx and Rx */
		dai_update_bits(dai, REG_SAI_TCSR, REG_SAI_CSR_SR,
				REG_SAI_CSR_SR);
		dai_update_bits(dai, REG_SAI_RCSR, REG_SAI_CSR_SR,
				REG_SAI_CSR_SR);

		/* Clear SR bit to finish the reset */
		dai_update_bits(dai, REG_SAI_TCSR, REG_SAI_CSR_SR, 0U);
		dai_update_bits(dai, REG_SAI_RCSR, REG_SAI_CSR_SR, 0U);
	}
}

static int sai_context_store(struct dai *dai)
{
	return 0;
}

static int sai_context_restore(struct dai *dai)
{
	return 0;
}

static inline int sai_set_config(struct dai *dai,
				 struct sof_ipc_dai_config *config)
{
	dai_info(dai, "SAI: sai_set_config");
	uint32_t val_cr2 = 0, val_cr4 = 0, val_cr5 = 0;
	uint32_t mask_cr2 = 0, mask_cr4 = 0, mask_cr5 = 0;
	/* TODO: this value will be provided by config */
	uint32_t sywd = 32;

	switch (config->format & SOF_DAI_FMT_FORMAT_MASK) {
	case SOF_DAI_FMT_I2S:
		/*
		 * Frame low, 1clk before data, one word length for frame sync,
		 * frame sync starts one serial clock cycle earlier,
		 * that is, together with the last bit of the previous
		 * data word.
		 */
		val_cr2 |= REG_SAI_CR2_BCP;
		val_cr4 |= REG_SAI_CR4_FSE | REG_SAI_CR4_FSP;
		val_cr4 |= REG_SAI_CR4_SYWD(sywd);
		val_cr4 |= REG_SAI_CR4_MF;
		val_cr4 |= REG_SAI_CR4_FSE;
		break;
	case SOF_DAI_FMT_LEFT_J:
		/*
		 * Frame high, one word length for frame sync,
		 * frame sync asserts with the first bit of the frame.
		 */
		val_cr2 |= REG_SAI_CR2_BCP;
		val_cr4 |= REG_SAI_CR4_SYWD(sywd);
		val_cr4 |= REG_SAI_CR4_MF;
		break;
	case SOF_DAI_FMT_DSP_A:
		/*
		 * Frame high, 1clk before data, one bit for frame sync,
		 * frame sync starts one serial clock cycle earlier,
		 * that is, together with the last bit of the previous
		 * data word.
		 */
		val_cr2 |= REG_SAI_CR2_BCP;
		val_cr4 |= REG_SAI_CR4_FSE;
		val_cr4 |= REG_SAI_CR4_SYWD(0U);
		val_cr4 |= REG_SAI_CR4_MF;
		val_cr4 |= REG_SAI_CR4_FSE;
		break;
	case SOF_DAI_FMT_DSP_B:
		/*
		 * Frame high, one bit for frame sync,
		 * frame sync asserts with the first bit of the frame.
		 */
		val_cr2 |= REG_SAI_CR2_BCP;
		val_cr4 |= REG_SAI_CR4_SYWD(0U);
		val_cr4 |= REG_SAI_CR4_MF;
		break;
	case SOF_DAI_FMT_PDM:
		val_cr2 |= REG_SAI_CR2_BCP;
		val_cr4 &= ~REG_SAI_CR4_MF;
		val_cr4 |= REG_SAI_CR4_MF;
		break;
	case SOF_DAI_FMT_RIGHT_J:
		val_cr4 |= REG_SAI_CR4_SYWD(sywd);
		val_cr4 |= REG_SAI_CR4_MF;
		break;
	default:
		return -EINVAL;
	}

	/* DAI clock inversion */
	switch (config->format & SOF_DAI_FMT_INV_MASK) {
	case SOF_DAI_FMT_IB_IF:
		/* Invert both clocks */
		val_cr2 ^= REG_SAI_CR2_BCP;
		val_cr4 ^= REG_SAI_CR4_FSP;
		break;
	case SOF_DAI_FMT_IB_NF:
		/* Invert bit clock */
		val_cr2 ^= REG_SAI_CR2_BCP;
		break;
	case SOF_DAI_FMT_NB_IF:
		/* Invert frame clock */
		val_cr4 ^= REG_SAI_CR4_FSP;
		break;
	case SOF_DAI_FMT_NB_NF:
		/* Nothing to do for both normal cases */
		break;
	default:
		return -EINVAL;
	}

	/* DAI clock master masks */
	switch (config->format & SOF_DAI_FMT_MASTER_MASK) {
	case SOF_DAI_FMT_CBS_CFS:
		dai_info(dai, "SAI: codec is slave");
		val_cr2 |= REG_SAI_CR2_MSEL_MCLK1;
		val_cr2 |= REG_SAI_CR2_BCD_MSTR;
		val_cr2 |= SAI_CLOCK_DIV; /* TODO: determine dynamically.*/
		val_cr4 |= REG_SAI_CR4_FSD_MSTR;
		break;
	case SOF_DAI_FMT_CBM_CFM:
		dai_info(dai, "SAI: codec is master");
		/*
		 * fields CR2_DIV and CR2_MSEL not relevant in slave mode.
		 * fields CR2_BCD and CR4_MFSD already at 0
		 */
		break;
	case SOF_DAI_FMT_CBS_CFM:
		val_cr2 |= REG_SAI_CR2_BCD_MSTR;
		val_cr2 |= SAI_CLOCK_DIV; /* TODO: determine dynamically.*/
		break;
	case SOF_DAI_FMT_CBM_CFS:
		val_cr4 |= REG_SAI_CR4_FSD_MSTR;
		val_cr2 |= SAI_CLOCK_DIV; /* TODO: determine dynamically.*/
		break;
	default:
		return -EINVAL;
	}

	/* TODO: set number of slots from config */
	val_cr4 |= REG_SAI_CR4_FRSZ(SAI_TDM_SLOTS);
	val_cr4 |= REG_SAI_CR4_CHMOD;

	val_cr5 |= REG_SAI_CR5_WNW(sywd) | REG_SAI_CR5_W0W(sywd) |
			REG_SAI_CR5_FBT(sywd);

	mask_cr2  = REG_SAI_CR2_BCP | REG_SAI_CR2_BCD_MSTR |
			REG_SAI_CR2_MSEL_MASK | REG_SAI_CR2_DIV_MASK;

	mask_cr4  = REG_SAI_CR4_MF | REG_SAI_CR4_FSE |
			REG_SAI_CR4_FSP | REG_SAI_CR4_FSD_MSTR |
			REG_SAI_CR4_FRSZ_MASK | REG_SAI_CR4_SYWD_MASK |
			REG_SAI_CR4_CHMOD_MASK;


	mask_cr5  = REG_SAI_CR5_WNW_MASK | REG_SAI_CR5_W0W_MASK |
			REG_SAI_CR5_FBT_MASK;

	dai_update_bits(dai, REG_SAI_XCR2(REG_TX_DIR), mask_cr2, val_cr2);
	dai_update_bits(dai, REG_SAI_XCR4(REG_TX_DIR), mask_cr4, val_cr4);
	dai_update_bits(dai, REG_SAI_XCR5(REG_TX_DIR), mask_cr5, val_cr5);
	/* turn on (set to zero) stereo slot */
	dai_update_bits(dai, REG_SAI_XMR(REG_TX_DIR),  REG_SAI_XMR_MASK,
			~(BIT(0) | BIT(1)));

	val_cr2 |= REG_SAI_CR2_SYNC;
	mask_cr2 |= REG_SAI_CR2_SYNC_MASK;
	dai_update_bits(dai, REG_SAI_XCR2(REG_RX_DIR), mask_cr2, val_cr2);
	dai_update_bits(dai, REG_SAI_XCR4(REG_RX_DIR), mask_cr4, val_cr4);
	dai_update_bits(dai, REG_SAI_XCR5(REG_RX_DIR), mask_cr5, val_cr5);
	/* turn on (set to zero) stereo slot */
	dai_update_bits(dai, REG_SAI_XMR(REG_RX_DIR), REG_SAI_XMR_MASK,
			~(BIT(0) | BIT(1)));

	return 0;
}

static int sai_trigger(struct dai *dai, int cmd, int direction)
{
	dai_info(dai, "SAI: sai_trigger");

	switch (cmd) {
	case COMP_TRIGGER_START:
		sai_start(dai, direction);
		break;
	case COMP_TRIGGER_STOP:
		sai_stop(dai, direction);
		break;
	case COMP_TRIGGER_PAUSE:
		sai_stop(dai, direction);
		break;
	case COMP_TRIGGER_RELEASE:
		break;
	case COMP_TRIGGER_SUSPEND:
		break;
	case COMP_TRIGGER_RESUME:
		break;
	default:
		dai_err(dai, "SAI: invalid trigger cmd %d", cmd);
		break;
	}
	return 0;
}

static void sai_handler(void *data)
{
	struct dai *dai = data;
	uint32_t tcsr = dai_read(dai, REG_SAI_TCSR);
	uint32_t mask = (FSL_SAI_FLAGS >> FSL_SAI_CSR_xIE_SHIFT) << FSL_SAI_CSR_xF_SHIFT;
	uint32_t flags = tcsr & mask;
	static int count = 0;
	
	if (count < 5)
		dai_err(dai, "IRQ count %d tcsr %08x flags %08x", count, tcsr, flags);

	if(!flags)
		goto irq_rx;

	if (flags & REG_SAI_CSR_FEF)
		tcsr |= REG_SAI_CSR_FR | REG_SAI_CSR_SR;
 
	flags &= FSL_SAI_CSR_xF_W_MASK;
	tcsr &= ~FSL_SAI_CSR_xF_MASK;

	if (count % 50  == 0)
		dai_err(dai, "IRQ count %d tcsr %08x flags %08x -> | %08x", count, tcsr, flags, flags | tcsr);

	if(flags)
		dai_write(dai, REG_SAI_TCSR, flags | tcsr);

	dai_update_bits(dai, REG_SAI_TCSR, REG_SAI_CSR_SR, REG_SAI_CSR_SR);
	dai_update_bits(dai, REG_SAI_RCSR, REG_SAI_CSR_SR, REG_SAI_CSR_SR);

	/* Clear SR bit to finish the reset */
	dai_update_bits(dai, REG_SAI_TCSR, REG_SAI_CSR_SR, 0U);
	dai_update_bits(dai, REG_SAI_RCSR, REG_SAI_CSR_SR, 0U);

	count++;
	//	write32(sai_addr + FSL_SAI_TCSR(offset), flags | tcsr);

irq_rx:
#if 0
	rcsr = *(volatile unsigned int *)(sai_addr + FSL_SAI_RCSR(offset));
	flags = rcsr & mask;

	if (!flags)
		goto out;
	
	if (flags & FSL_SAI_CSR_FEF)
		rcsr |= FSL_SAI_CSR_FR;

	flags &= FSL_SAI_CSR_xF_W_MASK;
	tcsr &= ~FSL_SAI_CSR_xF_MASK;

	if(flags)
		write32(sai_addr + FSL_SAI_RCSR(offset), flags | tcsr);
out:
#endif
	return;
}

int irq;

static int sai_probe(struct dai *dai)
{
	dai_info(dai, "SAI: sai_probe");

	/* Software Reset for both Tx and Rx */
	dai_update_bits(dai, REG_SAI_TCSR, REG_SAI_CSR_SR, REG_SAI_CSR_SR);
	dai_update_bits(dai, REG_SAI_RCSR, REG_SAI_CSR_SR, REG_SAI_CSR_SR);

	/* Clear SR bit to finish the reset */
	dai_update_bits(dai, REG_SAI_TCSR, REG_SAI_CSR_SR, 0U);
	dai_update_bits(dai, REG_SAI_RCSR, REG_SAI_CSR_SR, 0U);

	/* Reset all Tx register values */
	dai_write(dai, REG_SAI_TCR2, 0U);
	dai_write(dai, REG_SAI_TCR3, 0U);
	dai_write(dai, REG_SAI_TCR4, 0U);
	dai_write(dai, REG_SAI_TCR5, 0U);
	dai_write(dai, REG_SAI_TMR,  0U);

	/* Reset all Rx register values */
	dai_write(dai, REG_SAI_RCR2, 0U);
	dai_write(dai, REG_SAI_RCR3, 0U);
	dai_write(dai, REG_SAI_RCR4, 0U);
	dai_write(dai, REG_SAI_RCR5, 0U);
	dai_write(dai, REG_SAI_RMR,  0U);

	irq = interrupt_get_irq(18, "irqsteer1"); //SAI IRQ 50

	interrupt_register(irq, sai_handler, dai);

	interrupt_unmask(irq, cpu_get_id());
	interrupt_enable(irq, dai);
	
	return 0;
}

static int sai_get_handshake(struct dai *dai, int direction, int stream_id)
{
	return dai->plat_data.fifo[direction].handshake;
}

static int sai_get_fifo(struct dai *dai, int direction, int stream_id)
{
	switch (direction) {
	case DAI_DIR_PLAYBACK:
	case DAI_DIR_CAPTURE:
		return dai_fifo(dai, direction); /* stream_id is unused */
	default:
		dai_err(dai, "sai_get_fifo(): Invalid direction");
		return -EINVAL;
	}
}

static int sai_get_hw_params(struct dai *dai,
			     struct sof_ipc_stream_params *params,
			     int dir)
{
	/* SAI only currently supports these parameters */
	params->rate = 48000;
	params->channels = 2;
	params->buffer_fmt = 0;
	params->frame_fmt = SOF_IPC_FRAME_S32_LE;

	return 0;
}

const struct dai_driver sai_driver = {
	.type = SOF_DAI_IMX_SAI,
	.uid = SOF_UUID(sai_uuid),
	.tctx = &sai_tr,
	.dma_dev = DMA_DEV_SAI,
	.ops = {
		.trigger		= sai_trigger,
		.set_config		= sai_set_config,
		.pm_context_store	= sai_context_store,
		.pm_context_restore	= sai_context_restore,
		.probe			= sai_probe,
		.get_handshake		= sai_get_handshake,
		.get_fifo		= sai_get_fifo,
		.get_hw_params		= sai_get_hw_params,
	},
};
