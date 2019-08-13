// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright 2019 NXP
//
// Author: Daniel Baluta <daniel.baluta@nxp.com>

#include <sof/drivers/sai.h>
#include <sof/lib/dai.h>
#include <sof/lib/dma.h>
#include <ipc/dai.h>

static void sai_start(struct dai *dai, int dir)
{
	/* Tx/Rx enable */
	dai_update_bits(dai, FSL_SAI_xCSR(dir), FSL_SAI_CSR_TERE,
		FSL_SAI_CSR_TERE);

	/* FIFO Interrupt enable */
	dai_update_bits(dai, FSL_SAI_xCSR(dir), FSL_SAI_CSR_xIE_MASK,
			FSL_SAI_CSR_xIE_FLAGS);

	/* FIFO Request DMA Enable */
	dai_update_bits(dai, FSL_SAI_xCSR(dir), FSL_SAI_CSR_FRDE,
			FSL_SAI_CSR_FRDE);
}

static void sai_stop(struct dai *dai, int dir)
{
	/* FIFO Request DMA Disable */
	dai_update_bits(dai, FSL_SAI_xCSR(dir), FSL_SAI_CSR_FRDE, 0);

	/* FIFO Interrupt disable */
	dai_update_bits(dai, FSL_SAI_xCSR(dir), FSL_SAI_CSR_xIE_MASK, 0);

	/* Tx/Rx disable */
	dai_update_bits(dai, FSL_SAI_xCSR(dir), FSL_SAI_CSR_TERE, 0);

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
	uint32_t xcr4 = 0;
	uint32_t xcr5 = 0;
	uint32_t xcr4_mask;
	uint32_t xcr5_mask;
	uint32_t dir = 1; /* tx */

	xcr4 |= FSL_SAI_CR4_SYWD(config->sai.tdm_slot_width);
	xcr4 |= FSL_SAI_CR4_FRSZ(config->sai.tdm_slots);

	xcr4_mask = FSL_SAI_CR4_SYWD_MASK | FSL_SAI_CR4_FRSZ_MASK;

	xcr5 |= FSL_SAI_CR5_WNW(config->sai.tdm_slot_width);
	xcr5 |= FSL_SAI_CR5_WOW(config->sai.tdm_slot_width);
	xcr5 |= FSL_SAI_CR5_FBT(config->sai.tdm_slot_width - 1);

	xcr5_mask = FSL_SAI_CR5_WNW_MASK | FSL_SAI_CR5_WOW_MASK |
		FSL_SAI_CR5_FBT_MASK;

	dai_update_bits(dai, FSL_SAI_xCR4(dir), xcr4_mask, xcr4);
	dai_update_bits(dai, FSL_SAI_xCR5(dir), xcr5_mask, xcr5);

	dai_write(dai, FSL_SAI_xMR(dir),
		  ~0UL - ((1 << config->sai.tdm_slots) - 1));

	return 0;
}

static int sai_trigger(struct dai *dai, int cmd, int dir)
{

	switch(cmd) {
	case COMP_TRGGER_START:
	case COMP_TRIGGER_RELEASE:
		sai_start(dai, dir);
		break;
	case COMP_TRIGGER_STOP:
	case COMP_TRIGGER_PAUSE:
		sai_stop(dai, dir);
		break;
	default:
		break;
	}
	return 0;
}

static int sai_probe(struct dai *dai)
{
	return 0;
}

const struct dai_driver sai_driver = {
	.type = SOF_DAI_IMX_SAI,
	.dma_dev = DMA_DEV_SAI,
	.ops = {
		.trigger		= sai_trigger,
		.set_config		= sai_set_config,
		.pm_context_store	= sai_context_store,
		.pm_context_restore	= sai_context_restore,
		.probe			= sai_probe,
	},
};
