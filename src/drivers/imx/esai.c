/*
 * Copyright (c) 2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Intel Corporation nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Liam Girdwood <liam.r.girdwood@linux.intel.com>
 *         Keyon Jie <yang.jie@linux.intel.com>
 */

#include <errno.h>
#include <stdbool.h>
#include <sof/stream.h>
#include <sof/esai.h>
#include <sof/alloc.h>
#include <sof/interrupt.h>
#include <sof/drivers/printf.h>
#include <sof/drivers/peripheral.h>

/* tracing */
#define trace_ssp(__e, ...)	trace_event(TRACE_CLASS_SSP, __e, ##__VA_ARGS__)
#define trace_ssp_error(__e, ...)	trace_error(TRACE_CLASS_SSP, __e, ##__VA_ARGS__)
#define tracev_ssp(__e, ...)	tracev_event(TRACE_CLASS_SSP, __e, ##__VA_ARGS__)

#if 0
/* FIXME: move this to a helper and optimize */
static int hweight_32(uint32_t mask)
{
	int i;
	int count = 0;

	for (i = 0; i < 32; i++) {
		count += mask & 1;
		mask >>= 1;
	}
	return count;
}
#endif

#if 0
/* empty SSP receive FIFO */
static void ssp_empty_rx_fifo(struct dai *dai)
{
	uint32_t sssr;
	uint32_t entries;
	uint32_t i;

	spin_lock(&dai->lock);

	sssr = ssp_read(dai, SSSR);

	/* clear interrupt */
	if (sssr & SSSR_ROR)
		ssp_write(dai, SSSR, sssr);

	/* empty fifo */
	if (sssr & SSSR_RNE) {
		entries = SFIFOL_RFL(ssp_read(dai, SFIFOL));
		for (i = 0; i < entries + 1; i++)
			ssp_read(dai, SSDR);
	}

	spin_unlock(&dai->lock);
}
#endif

/* save ESAI context prior to entering D3 */
static int esai_context_store(struct dai *dai)
{
	return 0;
}

/* restore SSP context after leaving D3 */
static int esai_context_restore(struct dai *dai)
{
	return 0;
}

/* Digital Audio interface formatting */
static inline int esai_set_config(struct dai *dai,
				 struct sof_ipc_dai_config *config)
{
	struct esai_pdata *esai = dai_get_drvdata(dai);
	int ret = 0;

	__dsp_printf("esai set config , format %x\n", config->format);
	return 0;

	spin_lock(&dai->lock);

	/* is playback/capture already running */
	if (esai->state[DAI_DIR_PLAYBACK] == COMP_STATE_ACTIVE ||
	    esai->state[DAI_DIR_CAPTURE] == COMP_STATE_ACTIVE) {
		__dsp_printf("esai_set_config(): "
				"playback/capture already running");
		ret = -EINVAL;
		goto out;
	}

	__dsp_printf("esai_set_config(), config->format = %d", config->format);

	switch (config->format & SOF_DAI_FMT_MASTER_MASK) {
	case SOF_DAI_FMT_CBM_CFM:
		/*TODO: */
		break;
	case SOF_DAI_FMT_CBS_CFS:
		/* TODO */
		break;
	case SOF_DAI_FMT_CBM_CFS:
		/* TODO: */
		break;
	case SOF_DAI_FMT_CBS_CFM:
		/* TODO : */
		break;
	default:
		__dsp_printf("esai_set_config() error: "
				"format & MASTER_MASK EINVAL");
		ret = -EINVAL;
		goto out;
	}

	/* clock signal polarity */
	switch (config->format & SOF_DAI_FMT_INV_MASK) {
	case SOF_DAI_FMT_NB_NF:
		break;
	case SOF_DAI_FMT_NB_IF:
		break;
	case SOF_DAI_FMT_IB_IF:
		break;
	case SOF_DAI_FMT_IB_NF:
		break;
	default:
		__dsp_printf("esai_set_config() error: "
				"format & INV_MASK EINVAL");
		ret = -EINVAL;
		goto out;
	}

	/* BCLK is generated from MCLK - must be divisable */

	/* format */
	switch (config->format & SOF_DAI_FMT_FORMAT_MASK) {
	case SOF_DAI_FMT_I2S:
		break;
	case SOF_DAI_FMT_LEFT_J:
		break;
	case SOF_DAI_FMT_DSP_A:
		break;
	case SOF_DAI_FMT_DSP_B:
		break;
	default:
		__dsp_printf("esai_set_config() error: "
				"format & FORMAT_MASK EINVAL");
		ret = -EINVAL;
		goto out;
	}

	esai->state[DAI_DIR_PLAYBACK] = COMP_STATE_PREPARE;
	esai->state[DAI_DIR_CAPTURE] = COMP_STATE_PREPARE;

	esai_write(dai, REG_ESAI_ECR, ESAI_ECR_ERST);
	esai_write(dai, REG_ESAI_ECR, ESAI_ECR_ESAIEN);
	esai_write(dai, REG_ESAI_SAICR, 0);

	esai_update_bits(dai, REG_ESAI_TCCR,  ESAI_xCCR_xDC_MASK,
		    ESAI_xCCR_xDC(2));
	esai_update_bits(dai, REG_ESAI_TCCR,
		    ESAI_xCCR_xCKP | ESAI_xCCR_xHCKP | ESAI_xCCR_xHCKD |
		    ESAI_xCCR_xFSP | ESAI_xCCR_xFSD | ESAI_xCCR_xCKD,
		    ESAI_xCCR_xCKP | ESAI_xCCR_xHCKP | ESAI_xCCR_xHCKD |
		    ESAI_xCCR_xFSD | ESAI_xCCR_xCKD);

	esai_update_bits(dai, REG_ESAI_TCR,
		    ESAI_xCR_xFSL | ESAI_xCR_xFSR | ESAI_xCR_xWA, 0);

//	if (channels > 1)
		esai_update_bits(dai, REG_ESAI_TCR,
			    ESAI_xCR_xMOD_MASK, ESAI_xCR_xMOD_NETWORK);
//	else
//		esai_update_bits(dai, REG_ESAI_TCR, ESAI_xCR_xMOD_MASK, 0);

	esai_update_bits(dai, REG_ESAI_TCR,
				ESAI_xCR_xSWS_MASK | ESAI_xCR_PADC,
				ESAI_xCR_xSWS(32, 16) | ESAI_xCR_PADC);

	esai_update_bits(dai, REG_ESAI_PRRC,
				ESAI_PRRC_PDC_MASK,
				ESAI_PRRC_PDC(ESAI_GPIO));
        esai_update_bits(dai, REG_ESAI_PCRC,
				ESAI_PCRC_PC_MASK,
				ESAI_PCRC_PC(ESAI_GPIO));

        esai_write(dai, REG_ESAI_TSMA, 0);
        esai_write(dai, REG_ESAI_TSMB, 0);
	esai_write(dai, REG_ESAI_RSMA, 0);
	esai_write(dai, REG_ESAI_RSMB, 0);

	esai_update_bits(dai, REG_ESAI_ECR, ESAI_ECR_ETI, ESAI_ECR_ETI);

	esai_update_bits(dai, REG_ESAI_TFCR,
				ESAI_xFCR_xFR_MASK,
				ESAI_xFCR_xFR);

	esai_update_bits(dai, REG_ESAI_TFCR,
				ESAI_xFCR_xFR_MASK | ESAI_xFCR_xWA_MASK | ESAI_xFCR_xFWM_MASK | ESAI_xFCR_TE_MASK | ESAI_xFCR_TIEN,
				ESAI_xFCR_xWA(16) | ESAI_xFCR_xFWM(64) | ESAI_xFCR_TE(1) | ESAI_xFCR_TIEN);

	esai_update_bits(dai, REG_ESAI_TCCR, ESAI_xCCR_xFP_MASK, ESAI_xCCR_xFP(8));
	esai_update_bits(dai, REG_ESAI_TCCR, ESAI_xCCR_xPSR_MASK, ESAI_xCCR_xPSR_BYPASS);
	__dsp_printf("esai_set_config(), done");

out:
	spin_unlock(&dai->lock);

	return ret;
}

#if 0
/* Digital Audio interface formatting */
static inline int ssp_set_loopback_mode(struct dai *dai, uint32_t lbm)
{
	trace_ssp("ssp_set_loopback_mode()");
	spin_lock(&dai->lock);

	ssp_update_bits(dai, SSCR1, SSCR1_LBM, lbm ? SSCR1_LBM : 0);

	spin_unlock(&dai->lock);

	return 0;
}
#endif

/* start the ESAI for either playback or capture */
static void esai_start(struct dai *dai, int direction)
{
	uint32_t tx = direction;
	struct esai_pdata *esai = dai_get_drvdata(dai);


	__dsp_printf("esai start\n");
	spin_lock(&dai->lock);

	/* enable port */
	esai->state[direction] = COMP_STATE_ACTIVE;

	esai_update_bits(dai, REG_ESAI_xFCR(tx), ESAI_xFCR_xFEN_MASK, ESAI_xFCR_xFEN);
	esai_update_bits(dai, REG_ESAI_xCR(tx),
				tx ? ESAI_xCR_TE_MASK : ESAI_xCR_RE_MASK,
				tx ? ESAI_xCR_TE(1) : ESAI_xCR_RE(1));
	esai_update_bits(dai, REG_ESAI_xSMB(tx),
				ESAI_xSMB_xS_MASK, ESAI_xSMB_xS(0x3));
	esai_update_bits(dai, REG_ESAI_xSMA(tx),
				ESAI_xSMA_xS_MASK, ESAI_xSMA_xS(0x3));

	spin_unlock(&dai->lock);
}

/* stop the esai for either playback or capture */
static void esai_stop(struct dai *dai, int direction)
{
	struct esai_pdata *esai = dai_get_drvdata(dai);
	uint32_t tx = direction;

	spin_lock(&dai->lock);

	/* stop Rx if neeed */
	if (direction == DAI_DIR_CAPTURE &&
	    esai->state[SOF_IPC_STREAM_CAPTURE] == COMP_STATE_ACTIVE) {
		esai->state[SOF_IPC_STREAM_CAPTURE] = COMP_STATE_PAUSED;
		__dsp_printf("esai_stop(), RX stop");
	}

	/* stop Tx if needed */
	if (direction == DAI_DIR_PLAYBACK &&
	    esai->state[SOF_IPC_STREAM_PLAYBACK] == COMP_STATE_ACTIVE) {
		esai->state[SOF_IPC_STREAM_PLAYBACK] = COMP_STATE_PAUSED;
		trace_ssp("esai_stop(), TX stop");
	}

	/* disable esai port if no users */
	if (esai->state[SOF_IPC_STREAM_CAPTURE] != COMP_STATE_ACTIVE &&
	    esai->state[SOF_IPC_STREAM_PLAYBACK] != COMP_STATE_ACTIVE) {
		esai->state[SOF_IPC_STREAM_CAPTURE] = COMP_STATE_PREPARE;
		esai->state[SOF_IPC_STREAM_PLAYBACK] = COMP_STATE_PREPARE;
		trace_ssp("esai_stop(), ESAI port disabled");
	}

	esai_update_bits(dai, REG_ESAI_xCR(tx),
				tx ? ESAI_xCR_TE_MASK : ESAI_xCR_RE_MASK, 0);
	esai_update_bits(dai, REG_ESAI_xSMA(tx),
				ESAI_xSMA_xS_MASK, 0);
        esai_update_bits(dai, REG_ESAI_xSMB(tx),
				ESAI_xSMB_xS_MASK, 0);
	esai_update_bits(dai, REG_ESAI_xFCR(tx),
				ESAI_xFCR_xFR | ESAI_xFCR_xFEN,
				ESAI_xFCR_xFR);
	esai_update_bits(dai, REG_ESAI_xFCR(tx),
				ESAI_xFCR_xFR, 0);

	spin_unlock(&dai->lock);
}

static int esai_trigger(struct dai *dai, int cmd, int direction)
{
	__dsp_printf("esai trigger, cmd %x direction %d\n", cmd, direction);
	struct esai_pdata *esai = dai_get_drvdata(dai);

	trace_ssp("ssp_trigger()");

	switch (cmd) {
	case COMP_TRIGGER_START:
		if (esai->state[direction] == COMP_STATE_PREPARE ||
		    esai->state[direction] == COMP_STATE_PAUSED)
			esai_start(dai, direction);
		break;
	case COMP_TRIGGER_RELEASE:
		if (esai->state[direction] == COMP_STATE_PAUSED ||
		    esai->state[direction] == COMP_STATE_PREPARE)
			esai_start(dai, direction);
		break;
	case COMP_TRIGGER_STOP:
	case COMP_TRIGGER_PAUSE:
		esai_stop(dai, direction);
		break;
	case COMP_TRIGGER_RESUME:
		esai_context_restore(dai);
		break;
	case COMP_TRIGGER_SUSPEND:
		esai_context_store(dai);
		break;
	default:
		break;
	}
	return 0;
}
#if 0
/* clear IRQ sources atm */
static void esai_irq_handler(void *data)
{
	struct dai *dai = data;

	//trace_ssp("esai_irq_handler(), IRQ = %u", ssp_read(dai, SSSR));

	/* clear IRQ */
	//ssp_write(dai, SSSR, ssp_read(dai, SSSR));
	platform_interrupt_clear(esai_irq(dai), 1);
}
#endif

static int esai_probe(struct dai *dai)
{
	//int ret;
	struct esai_pdata *esai;

	__dsp_printf("esai probe\n");
	/* allocate private data */
	esai = rzalloc(RZONE_SYS | RZONE_FLAG_UNCACHED, SOF_MEM_CAPS_RAM,
		      sizeof(*esai));
	if (!esai) {
		__dsp_printf("esai_probe error\n");
		return -ENOMEM;
	}

	dai_set_drvdata(dai, esai);

	spinlock_init(&dai->lock);

	esai->state[DAI_DIR_PLAYBACK] = COMP_STATE_READY;
	esai->state[DAI_DIR_CAPTURE] = COMP_STATE_READY;

#if 0
	/* register our IRQ handler */
	ret = interrupt_register(esai_irq(dai), IRQ_AUTO_UNMASK,
				 esai_irq_handler, dai);
	if (ret < 0) {
		//trace_ssp_error("SSP failed to allocate IRQ");
		rfree(esai);
		return ret;
	}

	platform_interrupt_unmask(esai_irq(dai), 1);
	interrupt_enable(esai_irq(dai));
#endif
	//ssp_empty_rx_fifo(dai);
	__dsp_printf("ESAI probe OK\n");

	return 0;
}

const struct dai_ops esai_ops = {
	.trigger		= esai_trigger,
	.set_config		= esai_set_config,
	.pm_context_store	= esai_context_store,
	.pm_context_restore	= esai_context_restore,
	.probe			= esai_probe,
#if 0
	.set_loopback_mode	= ssp_set_loopback_mode,
#endif
};
