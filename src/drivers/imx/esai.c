// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright 2019 NXP
//
// Author: Daniel Baluta <daniel.baluta@nxp.com>

#include <sof/drivers/esai.h>
#include <sof/lib/dai.h>
#include <sof/lib/dma.h>
#include <ipc/dai.h>

static int esai_context_store(struct dai *dai)
{
	return 0;
}

static int esai_context_restore(struct dai *dai)
{
	return 0;
}

static inline int esai_set_config(struct dai *dai,
				 struct sof_ipc_dai_config *config)
{
	return 0;
}

static void esai_start(struct dai *dai, int dir)
{
	/* dir FIFO enable */
	dai_update_bits(dai, REG_ESAI_xFCR(dir), ESAI_xFCR_xFEN_MASK,
			ESAI_xFCR_xFEN);


	dai_update_bits(dai, REG_ESAI_xCR(dir),
			dir ? ESAI_xCR_TE_MASK : ESAI_xCR_RE_MASK,
			dir ? ESAI_xCR_TE(1) : ESAI_xCR_RE(1));

	dai_update_bits(dai, REG_ESAI_xSMB(dir), ESAI_xSMB_xS_MASK,
			ESAI_xSMB_xS(0x3));

	dai_update_bits(dai, REG_ESAI_xSMA(dir), ESAI_xSMA_xS_MASK,
			ESAI_xSMA_xS(0x3));

	/* enable exception interrupt */
	dai_update_bits(dai, REG_ESAI_xCR(dir), ESAI_xCR_xEIE_MASK,
			ESAI_xCR_EIE);
}

static void esai_stop(struct dai *dai, int dir)
{
	/* disable exception interrupt */
	dai_update_bits(dai, REG_ESAI_xCR(dir), ESAI_xCR_xEIE_MASK, 0);

	dai_update_bits(dai, REG_ESAI_xSMA(dir), ESAI_xSMA_xS_MASK, 0);
	dai_update_bits(dai, REG_ESAI_xSMB(dir), ESAI_xSMB_xS_MASK, 0);

	dai_update_bits(dai, REG_ESAI_xCR(dir),
			dir ? ESAI_xCR_TE_MASK : ESAI_xCR_RE_MASK, 0);

	/* disable and reset FIFO */
	dai_update_bits(dai, REG_ESAI_xFCR(dir), ESAI_xFCR_xFR | ESAI_xFCR_xFEN,
			ESAI_xFCR_xFR);
	dai_update_bits(dai, REG_ESAI_xFCR(dir), ESAI_xFCR_xFR, 0);
}

static int esai_trigger(struct dai *dai, int cmd, int dir)
{
	switch(cmd) {
	case COMP_TRIGGER_START:
	case COMP_TRIGGER_RELEASE:
		esai_start(dai, dir);
		break;
	case COMP_TRIGGER_STOP:
	case COMP_TRIGGER_PAUSE:
		esai_stop(dai, dir);
		break;
	default:
		break;
	}

	return 0;
}

static int esai_probe(struct dai *dai)
{
	return 0;
}

const struct dai_driver esai_driver = {
	.type = SOF_DAI_IMX_ESAI,
	.dma_dev = DMA_DEV_ESAI,
	.ops = {
		.trigger		= esai_trigger,
		.set_config		= esai_set_config,
		.pm_context_store	= esai_context_store,
		.pm_context_restore	= esai_context_restore,
		.probe			= esai_probe,
	},
};
