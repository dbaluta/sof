// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2017 Intel Corporation. All rights reserved.
//
// Author: Seppo Ingalsuo <seppo.ingalsuo@linux.intel.com>

#include <sof/audio/component.h>
#include <sof/audio/format.h>
#include <rtos/alloc.h>
#include <sof/drivers/micfil.h>
#include <sof/lib/dai.h>
#include <sof/lib/dma.h>
#include <sof/lib/memory.h>
#include <sof/lib/pm_runtime.h>
#include <sof/lib/uuid.h>
#include <ipc/dai.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

LOG_MODULE_REGISTER(micfil_dai, CONFIG_SOF_LOG_LEVEL);

/* dd400475-35d7-4045-ab03-0c34957d7a08 */
DECLARE_SOF_UUID("micfil-dai", micfil_uuid, 0xdd400475, 0x35d7, 0x4045,
		 0xab, 0x03, 0x0c, 0x34, 0x95, 0x7d, 0x7a, 0x08);

DECLARE_TR_CTX(micfil_tr, SOF_UUID(micfil_uuid), LOG_LEVEL_INFO);

#define MICFIL_OSR_DEFAULT	16

enum quality {
	QUALITY_HIGH,
	QUALITY_MEDIUM,
	QUALITY_LOW,
	QUALITY_VLOW0,
	QUALITY_VLOW1,
	QUALITY_VLOW2,
};

static int micfil_reset(struct dai *dai)
{
	dai_update_bits(dai, REG_MICFIL_CTRL1, MICFIL_CTRL1_MDIS, 0);
	dai_update_bits(dai, REG_MICFIL_CTRL1, MICFIL_CTRL1_SRES, MICFIL_CTRL1_SRES);
	dai_update_bits(dai, REG_MICFIL_STAT, 0xff, 0xff);

	return 0;
}

static int micfil_get_hw_params(struct dai *dai,
			      struct sof_ipc_stream_params *params, int dir)
{
	struct micfil_pdata *micfil = dai_get_drvdata(dai);

	dai_err(dai, "micfil_get_hw_params()");

	params->rate = micfil->params.pdm_rate;
	params->channels = micfil->params.pdm_ch;
	params->buffer_fmt = SOF_IPC_BUFFER_INTERLEAVED;
	params->frame_fmt = SOF_IPC_FRAME_S32_LE;

	dai_err(dai, "micfil_get_hw_params() rate %d chan %d dir %d", params->rate, params->channels, dir);
	return 0;
}

static int micfil_set_quality(struct dai *dai)
{
	struct micfil_pdata *micfil = dai_get_drvdata(dai);
	int qsel;

	switch (micfil->quality) {
	case QUALITY_HIGH:
		qsel = MICFIL_QSEL_HIGH_QUALITY;
		break;
	case QUALITY_MEDIUM:
		qsel = MICFIL_QSEL_MEDIUM_QUALITY;
		break;
	case QUALITY_LOW:
		qsel = MICFIL_QSEL_LOW_QUALITY;
		break;
	case QUALITY_VLOW0:
		qsel = MICFIL_QSEL_VLOW0_QUALITY;
		break;
	case QUALITY_VLOW1:
		qsel = MICFIL_QSEL_VLOW1_QUALITY;
		break;
	case QUALITY_VLOW2:
		qsel = MICFIL_QSEL_VLOW2_QUALITY;
		break;
	default:
		return -EINVAL;
	}

	dai_update_bits(dai, REG_MICFIL_CTRL2, MICFIL_CTRL2_QSEL,
			MICFIL_CTRL2_QSEL_BITS(qsel));

	return 0;
}

static int get_pdm_clk(struct dai *dai, int rate)
{
	int osr;
	int pdm_clk = 0;
	unsigned int ctrl2_reg;

	ctrl2_reg = dai_read(dai, REG_MICFIL_CTRL2);

	osr = 16 - ((ctrl2_reg & MICFIL_CTRL2_CICOSR) >> MICFIL_CTRL2_CICOSR_SHIFT);

	//qsel = ctrl2_reg & MICFIL_CTRL2_QSEL;

	pdm_clk = rate * 4 * osr * 1;

	return pdm_clk;
}

static int micfil_get_clk_div(struct dai *dai, int rate)
{
	int64_t mclk_rate;
	int clk_div;
	int pdm_clk = 0;

	mclk_rate = 24576000;

	pdm_clk = get_pdm_clk(dai, rate);

	clk_div = mclk_rate / (pdm_clk * 2);

	return clk_div;
}

static int micfil_set_clock_params(struct dai *dai, int rate)
{
	int clk_div;
	dai_update_bits(dai, REG_MICFIL_CTRL2, MICFIL_CTRL2_CICOSR,
			MICFIL_CTRL2_CICOSR_BITS(MICFIL_OSR_DEFAULT));


	clk_div = micfil_get_clk_div(dai, rate);


	dai_err(dai, "micfil_set_clk_params: clk_div %d", clk_div);
	dai_update_bits(dai, REG_MICFIL_CTRL2,MICFIL_CTRL2_CLKDIV,
			MICFIL_CTRL2_CLKDIV_BITS(clk_div));

	return 0;
}


static int micfil_set_config(struct dai *dai, struct ipc_config_dai *common_config,
			   const void *spec_config)
{
	int val, i;
	int channels = 2;
//	int clk_div = 4;
//	int osr = MICFIL_OSR_DEFAULT;

	dai_info(dai, "micfil_set_config() idx %d %d",
		common_config->dai_index, common_config->sampling_frequency);

	/* disable the module */
	dai_update_bits(dai, REG_MICFIL_CTRL1, MICFIL_CTRL1_PDMIEN, 0);

	micfil_set_quality(dai);

#if 0
	dai_update_bits(dai, REG_MICFIL_CTRL2, MICFIL_CTRL2_CICOSR | MICFIL_CTRL2_CLKDIV,
			MICFIL_CTRL2_CICOSR_BITS(osr) |
			MICFIL_CTRL2_CLKDIV_BITS(clk_div));
#endif
	/* set default gain to 2 */
	dai_write(dai, REG_MICFIL_OUT_CTRL, 0x22222222);

	/* set DC Remover in bypass mode */
	val = 0;
	for (i = 0; i < 8; i++)
		val |= MICFIL_DC_BYPASS << MICFIL_DC_CHX_SHIFT(i);
	dai_update_bits(dai, REG_MICFIL_DC_CTRL, MICFIL_DC_CTRL_CONFIG, val);

	/* FIFO WMK */
	dai_update_bits(dai, REG_MICFIL_FIFO_CTRL, MICFIL_FIFO_CTRL_FIFOWMK,
		MICFIL_FIFO_CTRL_FIFOWMK_BITS(31));

	/* enable channels */
	dai_update_bits(dai, REG_MICFIL_CTRL1, 0xff, ((1 << channels) - 1));

	micfil_set_clock_params(dai, 48000);

	return 0;
}

static int micfil_start(struct dai *dai)
{
	int ret;

	dai_info(dai, "micfil_start()");

	ret = micfil_reset(dai);
	if (ret < 0) {
		dai_err(dai, "micfil_start(): failed to reset micfil");
		return ret;
	}
	
	/* DMA Interrupt Selection - DISEL bits
		* 00 - DMA and IRQ disabled
		* 01 - DMA req enabled
		* 10 - IRQ enabled
		* 11 - reserved
	*/
	dai_update_bits(dai, REG_MICFIL_CTRL1,
			MICFIL_CTRL1_DISEL,
			MICFIL_CTRL1_DISEL_BITS(MICFIL_CTRL1_DISEL_DMA));

	/* Enable the module */
	dai_update_bits(dai, REG_MICFIL_CTRL1, MICFIL_CTRL1_PDMIEN, MICFIL_CTRL1_PDMIEN);

	return 0;
}

static int micfil_stop(struct dai *dai)
{
	dai_info(dai, "micfil_stop()");

	/* Disable the module */
	dai_update_bits(dai, REG_MICFIL_CTRL1, MICFIL_CTRL1_PDMIEN, 0);

	dai_update_bits(dai, REG_MICFIL_CTRL1, MICFIL_CTRL1_DISEL,
		  MICFIL_CTRL1_DISEL_BITS(MICFIL_CTRL1_DISEL_DISABLE));
	return 0;
}


static int micfil_trigger(struct dai *dai, int cmd, int direction)
{
	dai_info(dai, "micfil_trigger() cmd %d dir %d", cmd, direction);

	switch (cmd) {
		case COMP_TRIGGER_START:
		case COMP_TRIGGER_RELEASE:
			micfil_start(dai);
			break;
		case COMP_TRIGGER_STOP:
		case COMP_TRIGGER_PAUSE:
			micfil_stop(dai);
		case COMP_TRIGGER_PRE_START:
		case COMP_TRIGGER_PRE_RELEASE:
			break;
		default:
			dai_err(dai, "MICFIL: invalid trigger cmd %d", cmd);
	}

	return 0;
}

static int micfil_probe(struct dai *dai)
{
	struct micfil_pdata *micfil;

	dai_info(dai, "micfil_probe()");

	micfil = rzalloc(SOF_MEM_ZONE_RUNTIME_SHARED, 0, SOF_MEM_CAPS_RAM, sizeof(*micfil));
	if (!micfil) {
		dai_err(dai, "micfil probe failed");
		return -ENOMEM;
	}

	micfil->quality = QUALITY_VLOW0;

	dai_set_drvdata(dai, micfil);

	return 0;
}

static int micfil_remove(struct dai *dai)
{
	struct micfil_pdata *micfil = dai_get_drvdata(dai);

	dai_info(dai, "micfil_remove()");

	rfree(micfil);
	dai_set_drvdata(dai, NULL);

	return 0;
}

static int micfil_get_handshake(struct dai *dai, int direction, int stream_id)
{
	return dai->plat_data.fifo[direction].handshake;
}

static int micfil_get_fifo(struct dai *dai, int direction, int stream_id)
{

	return dai->plat_data.fifo[direction].offset;
}

static int micfil_get_fifo_depth(struct dai *dai, int direction)
{
	return 128;
}

const struct dai_driver micfil_driver = {
	.type = SOF_DAI_IMX_MICFIL,
	.uid = SOF_UUID(micfil_uuid),
	.tctx = &micfil_tr,
	.dma_dev = DMA_DEV_MICFIL,
	.ops = {
		.trigger		= micfil_trigger,
		.set_config		= micfil_set_config,
		.get_hw_params		= micfil_get_hw_params,
		.get_handshake		= micfil_get_handshake,
		.get_fifo		= micfil_get_fifo,
		.get_fifo_depth		= micfil_get_fifo_depth,
		.probe			= micfil_probe,
		.remove			= micfil_remove,
//		.copy			= micfil_gain_ramp,
	},
};
