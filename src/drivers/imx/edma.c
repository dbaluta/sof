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
 *
 *
 * This driver API will only be called by 3 clients in sof :-
 *
 * 1. Host audio component. This component represents the ALSA PCM device
 *    and involves copying data to/from the host ALSA audio buffer to/from the
 *    the DSP buffer.
 *
 * 2. DAI audio component. This component represents physical DAIs and involves
 *    copying data to/from the DSP buffers to/from the DAI FIFOs.
 *
 * 3. IPC Layer. Some IPC needs DMA to copy audio buffer page table information
 *    from the host DRAM into DSP DRAM. This page table information is then
 *    used to construct the DMA configuration for the host client 1 above.
 */

#include <sof/atomic.h>
#include <sof/debug.h>
#include <sof/sof.h>
#include <sof/dma.h>
#include <sof/dw-dma.h>
#include <sof/io.h>
#include <sof/stream.h>
#include <sof/timer.h>
#include <sof/alloc.h>
#include <sof/interrupt.h>
#include <sof/work.h>
#include <sof/lock.h>
#include <sof/trace.h>
#include <sof/wait.h>
#include <sof/pm_runtime.h>
#include <sof/audio/component.h>
#include <sof/cpu.h>
#include <platform/dma.h>
#include <platform/platform.h>
#include <platform/interrupt.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <config.h>

/* channel registers */
#define EDMA_MAX_CHAN			8
#define DW_CH_SIZE			0x58
#define BYT_CHAN_OFFSET(chan) \
	(DW_CH_SIZE * chan)

#define DW_SAR(chan)	\
	(0x0000 + BYT_CHAN_OFFSET(chan))
#define DW_DAR(chan) \
	(0x0008 + BYT_CHAN_OFFSET(chan))
#define DW_LLP(chan) \
	(0x0010 + BYT_CHAN_OFFSET(chan))
#define DW_CTRL_LOW(chan) \
	(0x0018 + BYT_CHAN_OFFSET(chan))
#define DW_CTRL_HIGH(chan) \
	(0x001C + BYT_CHAN_OFFSET(chan))
#define DW_CFG_LOW(chan) \
	(0x0040 + BYT_CHAN_OFFSET(chan))
#define DW_CFG_HIGH(chan) \
	(0x0044 + BYT_CHAN_OFFSET(chan))
#define DW_DSR(chan) \
	(0x0050 + BYT_CHAN_OFFSET(chan))

/* registers */
#define DW_RAW_TFR			0x02C0
#define DW_RAW_BLOCK			0x02C8
#define DW_RAW_SRC_TRAN			0x02D0
#define DW_RAW_DST_TRAN			0x02D8
#define DW_RAW_ERR			0x02E0
#define DW_STATUS_TFR			0x02E8
#define DW_STATUS_BLOCK			0x02F0
#define DW_STATUS_SRC_TRAN		0x02F8
#define DW_STATUS_DST_TRAN		0x0300
#define DW_STATUS_ERR			0x0308
#define DW_MASK_TFR			0x0310
#define DW_MASK_BLOCK			0x0318
#define DW_MASK_SRC_TRAN		0x0320
#define DW_MASK_DST_TRAN		0x0328
#define DW_MASK_ERR			0x0330
#define DW_CLEAR_TFR			0x0338
#define DW_CLEAR_BLOCK			0x0340
#define DW_CLEAR_SRC_TRAN		0x0348
#define DW_CLEAR_DST_TRAN		0x0350
#define DW_CLEAR_ERR			0x0358
#define DW_INTR_STATUS			0x0360
#define DW_DMA_CFG			0x0398
#define DW_DMA_CHAN_EN			0x03A0

/* channel bits */
#define INT_MASK(chan)			(0x100 << chan)
#define INT_UNMASK(chan)		(0x101 << chan)
#define INT_MASK_ALL			0xFF00
#define INT_UNMASK_ALL			0xFFFF
#define CHAN_ENABLE(chan)		(0x101 << chan)
#define CHAN_DISABLE(chan)		(0x100 << chan)
#define CHAN_MASK(chan)		(0x1 << chan)

#define DW_CFG_CH_SUSPEND		0x100
#define DW_CFG_CH_FIFO_EMPTY		0x200

/* CTL_LO */
#define DW_CTLL_INT_EN			(1 << 0)
#define DW_CTLL_DST_WIDTH(x)		((x) << 1)
#define DW_CTLL_SRC_WIDTH(x)		((x) << 4)
#define DW_CTLL_DST_INC			(0 << 7)
#define DW_CTLL_DST_DEC			(1 << 7)
#define DW_CTLL_DST_FIX			(2 << 7)
#define DW_CTLL_SRC_INC			(0 << 9)
#define DW_CTLL_SRC_DEC			(1 << 9)
#define DW_CTLL_SRC_FIX			(2 << 9)
#define DW_CTLL_DST_MSIZE(x)		((x) << 11)
#define DW_CTLL_SRC_MSIZE(x)		((x) << 14)
#define DW_CTLL_FC(x)			((x) << 20)
#define DW_CTLL_FC_M2M			(0 << 20)
#define DW_CTLL_FC_M2P			(1 << 20)
#define DW_CTLL_FC_P2M			(2 << 20)
#define DW_CTLL_FC_P2P			(3 << 20)
#define DW_CTLL_DMS(x)			((x) << 23)
#define DW_CTLL_SMS(x)			((x) << 25)
#define DW_CTLL_LLP_D_EN		(1 << 27)
#define DW_CTLL_LLP_S_EN		(1 << 28)
#define DW_CTLL_RELOAD_SRC		(1 << 30)
#define DW_CTLL_RELOAD_DST		(1 << 31)

/* Haswell / Broadwell specific registers */
#if defined(CONFIG_HASWELL) || defined(CONFIG_BROADWELL)

/* CTL_HI */
#define DW_CTLH_DONE(x)			((x) << 12)
#define DW_CTLH_BLOCK_TS_MASK		0x00000fff

/* CFG_LO */
#define DW_CFG_CLASS(x)		(x << 5)

/* CFG_HI */
#define DW_CFGH_SRC_PER(x)		(x << 7)
#define DW_CFGH_DST_PER(x)		(x << 11)

/* default initial setup register values */
#define DW_CFG_LOW_DEF				0x0
#define DW_CFG_HIGH_DEF			0x4

#elif defined(CONFIG_BAYTRAIL) || defined(CONFIG_CHERRYTRAIL)
/* baytrail specific registers */

/* CTL_LO */
#define DW_CTLL_S_GATH_EN		(1 << 17)
#define DW_CTLL_D_SCAT_EN		(1 << 18)

/* CTL_HI */
#define DW_CTLH_DONE(x)			((x) << 17)
#define DW_CTLH_BLOCK_TS_MASK		0x0001ffff
#define DW_CTLH_CLASS(x)		((x) << 29)
#define DW_CTLH_WEIGHT(x)		((x) << 18)

/* CFG_LO */
#define DW_CFG_CH_DRAIN		0x400

/* CFG_HI */
#define DW_CFGH_SRC_PER(x)		((x) << 0)
#define DW_CFGH_DST_PER(x)		((x) << 4)

/* FIFO Partition */
#define DW_FIFO_PARTITION
#define DW_FIFO_PART0_LO		0x0400
#define DW_FIFO_PART0_HI		0x0404
#define DW_FIFO_PART1_LO		0x0408
#define DW_FIFO_PART1_HI		0x040C
#define DW_CH_SAI_ERR			0x0410

/* default initial setup register values */
#define DW_CFG_LOW_DEF			0x00000003
#define DW_CFG_HIGH_DEF		0x0

#elif defined(CONFIG_APOLLOLAKE) || defined(CONFIG_CANNONLAKE) || \
	defined(CONFIG_ICELAKE) || defined CONFIG_SUECREEK

/* CTL_LO */
#define DW_CTLL_S_GATH_EN		(1 << 17)
#define DW_CTLL_D_SCAT_EN		(1 << 18)

/* CTL_HI */
#define DW_CTLH_DONE(x)			((x) << 17)
#define DW_CTLH_BLOCK_TS_MASK		0x0001ffff
#define DW_CTLH_CLASS(x)		((x) << 29)
#define DW_CTLH_WEIGHT(x)		((x) << 18)

/* CFG_LO */
#define DW_CFG_CTL_HI_UPD_EN		(1 << 5)
#define DW_CFG_CH_DRAIN			(1 << 10)
#define DW_CFG_RELOAD_SRC		(1 << 30)
#define DW_CFG_RELOAD_DST		(1 << 31)

/* CFG_HI */
#if !defined(CONFIG_SUECREEK)
#define DW_CFGH_SRC_PER(x)		(x << 0)
#define DW_CFGH_DST_PER(x)		(x << 4)
#else
#define DW_CFGH_SRC_PER(x)	((((x) & 0xf) << 0) | (((x) & 0x30) << 24))
#define DW_CFGH_DST_PER(x)	((((x) & 0xf) << 4) | (((x) & 0x30) << 26))
#endif

/* FIFO Partition */
#define DW_FIFO_PARTITION
#define DW_FIFO_PART0_LO		0x0400
#define DW_FIFO_PART0_HI		0x0404
#define DW_FIFO_PART1_LO		0x0408
#define DW_FIFO_PART1_HI		0x040C
#define DW_CH_SAI_ERR			0x0410
#define DW_DMA_GLB_CFG			0x0418

/* default initial setup register values */
#define DW_CFG_LOW_DEF			0x00000003
#define DW_CFG_HIGH_DEF			0x0

#define DW_REG_MAX			DW_DMA_GLB_CFG
#endif

/* tracing */
#define trace_dwdma(__e, ...) \
	trace_event(TRACE_CLASS_DMA, __e, ##__VA_ARGS__)
#define trace_dwdma_atomic(__e, ...) \
	trace_event_atomic(TRACE_CLASS_DMA, __e, ##__VA_ARGS__)
#define tracev_dwdma(__e, ...) \
	tracev_event(TRACE_CLASS_DMA, __e, ##__VA_ARGS__)
#define trace_dwdma_error(__e, ...) \
	trace_error(TRACE_CLASS_DMA, __e, ##__VA_ARGS__)

/* HW Linked list support, only enabled for APL/CNL at the moment */
#if defined CONFIG_APOLLOLAKE || defined CONFIG_CANNONLAKE || \
	defined CONFIG_ICELAKE || defined CONFIG_SUECREEK
#define DW_USE_HW_LLI	1
#else
#define DW_USE_HW_LLI	0
#endif

/* number of tries to wait for reset */
#define DW_DMA_CFG_TRIES	10000

struct dma_id {
	struct dma *dma;
	uint32_t channel;
};

/* data for each DMA channel */
struct dma_chan_data {
	uint32_t status;
	uint32_t direction;
	uint32_t desc_count;
	uint32_t desc_avail;
	uint32_t period_bytes;
	uint32_t buffer_bytes;
	struct dma_id id;
	uint32_t timer_delay;
	struct work dma_ch_work;

	/* client callback function */
	void (*cb)(void *data, uint32_t type, struct dma_sg_elem *next);
	/* client callback data */
	void *cb_data;
	/* callback type */
	int cb_type;
};

/* private data for DW DMA engine */
struct dma_pdata {
	struct dma_chan_data chan[EDMA_MAX_CHAN];
	uint32_t class;		/* channel class - set for controller atm */
};

static inline void dw_dma_chan_reload_lli(struct dma *dma, int channel);
static inline void dw_dma_chan_reload_next(struct dma *dma, int channel,
					   struct dma_sg_elem *next);
static inline int dw_dma_interrupt_register(struct dma *dma, int channel);
static inline void dw_dma_interrupt_unregister(struct dma *dma, int channel);
static uint64_t dw_dma_work(void *data, uint64_t delay);

static inline void edma_write(struct dma *dma, uint32_t reg, uint32_t value)
{
	io_reg_write(dma_base(dma) + reg, value);
}

static inline uint32_t edma_read(struct dma *dma, uint32_t reg)
{
	return io_reg_read(dma_base(dma) + reg);
}

static inline void edma_write16(struct dma *dma, uint32_t reg, uint16_t value)
{
	io_reg_write16(dma_base(dma) + reg, value);
}

static inline uint16_t edma_read16(struct dma *dma, uint32_t reg)
{
	return io_reg_read16(dma_base(dma) + reg);
}

static inline void edma_update_bits(struct dma *dma, uint32_t reg, uint32_t mask,
				  uint32_t value)
{
	io_reg_update_bits(dma_base(dma) + reg, mask, value);
}


void edma_fill_tcd(struct nxp_edma_hw_tcd *tcd, uint32_t src_addr, uint32_t dst_addr,
		   uint16_t attr, uint16_t soff, uint32_t nbytes, uint32_t slast, uint16_t citer,
		   uint16_t biter, uint16_t doff, uint32_t dlast_sga, uint32_t major_int,
		   uint32_t disable_req, uint32_t enable_sg, uint32_t is_rx) {
	uint16_t csr = 0;

	tcd->saddr = src_addr;
	tcd->daddr = dst_addr;

	tcd->attr = attr;
	tcd->soff = soff;

	tcd->nbytes = nbytes;
	tcd->slast  = slast;

	tcd->citer  = citer;
	tcd->doff   = doff;

	tcd->dlast_sga = dlast_sga;
	tcd->biter     = biter;

	if (major_int)
		csr |= EDMA_TCD_CSR_INT_MAJOR;

	if (disable_req)
		csr |= EDMA_TCD_CSR_D_REQ;

	if (enable_sg)
		csr |= EDMA_TCD_CSR_E_SG;

	if (is_rx)
		csr |= EDMA_TCD_CSR_ACTIVE;

	tcd->csr = csr;
}

void edma_set_tcd(struct dma *dma, struct nxp_edma_hw_tcd *tcd) {

	edma_write(dma, EDMA_TCD_CSR,  0);
	edma_write(dma, EDMA_TCD_SADDR, tcd->saddr);
	edma_write(dma, EDMA_TCD_DADDR, tcd->daddr);
	edma_write16(dma, EDMA_TCD_ATTR, tcd->attr);
	edma_write16(dma, EDMA_TCD_SOFF, tcd->soff);
	edma_write(dma, EDMA_TCD_NBYTES, tcd->nbytes);
	edma_write(dma, EDMA_TCD_SLAST, tcd->slast);
	edma_write16(dma, EDMA_TCD_CITER, tcd->citer);
	edma_write16(dma, EDMA_TCD_BITER, tcd->biter);
	edma_write16(dma, EDMA_TCD_DOFF, tcd->doff);

	edma_write(edma_addr + EDMA_TCD_DLAST_SGA, tcd->dlast_sga);
	
	if ((EDMA_TCD_CSR_E_SG | tcd->csr) &&
		(EDMA_CH_CSR_DONE | read32(edma_addr + EDMA_CH_CSR)))
		edma_write(edma_addr + EDMA_CH_CSR, EDMA_CH_CSR_DONE);

	edma_write16(edma_addr + EDMA_TCD_CSR, tcd->csr);
}

#if 0
void edma_init(struct dma *dma, uint32_t type,
	       struct nxp_edma_hw_tcd *tcd,
	       volatile void *dev_addr,
	       volatile void *dev2_addr,
	       volatile void *dma_buf_addr,
	       uint32_t period_size, uint32_t period_count) {
	uint32_t dma_addr = (uint32_t)dma_buf_addr;
	uint32_t period_len = period_size;
	uint32_t buf_len = period_len * period_count;
	uint32_t dma_addr_next = dma_addr;
	uint32_t src_addr, dst_addr;
	uint32_t soff, doff, nbytes, attr, iter, last_sg;
	uint32_t burst=4, addr_width=2;
	int i;

	nbytes = burst * addr_width;
	attr = (EDMA_TCD_ATTR_SSIZE_16BIT | EDMA_TCD_ATTR_DSIZE_16BIT);
	iter = period_len / nbytes;

	for ( i = 0; i < period_count; i ++ ) {
		if (dma_addr_next >= dma_addr + buf_len)
			dma_addr_next = dma_addr;
		if (type == DMA_MEM_TO_DEV) {
			src_addr = dma_addr_next;
			dst_addr = (uint32_t)dev_addr;
			soff = 2;
			doff = 0;
		} else if (type == DMA_DEV_TO_MEM) {
			src_addr = (uint32_t)dev_addr;
			dst_addr = dma_addr_next;
			soff = 0;
			doff = 2;
		} else {
			/* DMA_DEV_TO_DEV */
			src_addr = (uint32_t)dev2_addr;
			dst_addr = (uint32_t)dev_addr;
			soff = 0;
			doff = 0;
		}

		last_sg = (uint32_t)&tcd[(i+1)%period_count];

		edma_fill_tcd(&tcd[i], src_addr, dst_addr, attr, soff, nbytes,
			      0, iter, iter, doff, last_sg, 1, 0, 1, 0);
		dma_addr_next = dma_addr_next + period_len;
	}
}
#endif

/* acquire the specific DMA channel */
static int edma_channel_get(struct dma *dma, int req_chan)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	uint32_t flags;
	int i;

	__dsp_printf("edma %d request channel %d\n", dma->plat_data.id, req_chan);

	if (req_chan >= EDMA_MAX_CHAN) {
		/* TODO: add trace warning */
		return -EINVAL;
	}

	spin_lock_irq(&dma->lock, flags);
	if (p->chan[req_chan].status == COMP_STATE_INIT) {
		p->chan[req_chan].status = COMP_STATE_READY;

		atomic_add(&dma->num_channels_busy, 1);
		spin_unlock_irq(&dma->lock, flags);
		__dsp_printf("Got channel %d\n", req_chan);
		return req_chan;
	}
	spin_unlock_irq(&dma->lock, flags);

	/* EDMA has no free channels */
	__dsp_printf("edma %d no free channel %d", dma->plat_data.id,
			  req_chan);
	return -ENODEV;
}

/* channel must not be running when this is called */
static void edma_channel_put_unlocked(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *chan = p->chan + channel;

	if (channel >= dma->plat_data.channels) {
		trace_dwdma_error("dw-dma: %d invalid channel %d",
				  dma->plat_data.id, channel);
		return;
	}

	/* set new state */
	chan->status = COMP_STATE_INIT;
	chan->state = 0;
	chan->period_bytes = 0;
	chan->buffer_bytes = 0;
	chan->cb = NULL;
	chan->cb_type = 0;
	chan->cb_data = NULL;
	chan->desc_count = 0;

	work_init(&chan->dma_ch_work, NULL, NULL, 0, 0);

	atomic_sub(&dma->num_channels_busy, 1);
}

/* channel must not be running when this is called */
static void edma_channel_put(struct dma *dma, int channel)
{
	uint32_t flags;

	trace_dwdma("dw-dma: %d channel %d put", dma->plat_data.id, channel);

	spin_lock_irq(&dma->lock, flags);
	edma_channel_put_unlocked(dma, channel);
	spin_unlock_irq(&dma->lock, flags);
}

static int edma_start(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *chan = p->chan + channel;
	struct fsl_edma_hw_tcd *tcd = chan->tcd_current;
	uint32_t flags, val;
	int start_offset = 0;
	int ret = 0;

	__dsp_printf("__edma start chan %d\n", channel);

	return 0;

	if (channel >= dma->plat_data.channels) {
		trace_dwdma_error("dw-dma: %d invalid channel %d",
				  dma->plat_data.id, channel);
		return -EINVAL;
	}

	tracev_dwdma("dw-dma: %d channel %d start", dma->plat_data.id, channel);

	/* is channel idle, disable and ready ? */
	if (chan->status != COMP_STATE_PREPARE) {
		/*XXX: add warn mesage, check if channel is disabled */
		return -EBUSY;
	}

	edma_set_tcd(edma_addr, tcd);

	val = edma_read(dma, EDMA_CH_SBR);

	if (chan->direction == DMA_DIR_DEV_TO_MEM)
		val |= EDMA_CH_SBR_RD;
	else
		val |= EDMA_CH_SBR_WR;

	edma_write(dma, EDMA_CH_SBR, val);

	val = edma_read(dma, EDMA_CH_CSR);
	val |= EDMA_CH_CSR_ERQ;
	edma_write(dma, EDMA_CH_CSR, val);

	return ret;
}

static int dw_dma_release(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	uint32_t flags;

	if (channel >= dma->plat_data.channels) {
		trace_dwdma_error("dw-dma: %d invalid channel %d",
				  dma->plat_data.id, channel);
		return -EINVAL;
	}
#if 0
	trace_dwdma("dw-dma: %d channel %d release", dma->plat_data.id, channel);
	spin_lock_irq(&dma->lock, flags);

	/* get next lli for proper release */
	p->chan[channel].tcd_current =
		(struct nxp_edma_hw_tcd *)p->chan[channel].tcd_current->llp;

	spin_unlock_irq(&dma->lock, flags);
#endif
	return 0;
}

static int dw_dma_pause(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	uint32_t flags;

	if (channel >= dma->plat_data.channels) {
		trace_dwdma_error("dw-dma: %d invalid channel %d",
				  dma->plat_data.id, channel);
		return -EINVAL;
	}

	trace_dwdma("dw-dma: %d channel %d pause", dma->plat_data.id, channel);

	spin_lock_irq(&dma->lock, flags);

	if (p->chan[channel].status != COMP_STATE_ACTIVE)
		goto out;

	/* pause the channel */
	p->chan[channel].status = COMP_STATE_PAUSED;

out:
	spin_unlock_irq(&dma->lock, flags);
	return 0;
}

static int edma_stop(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *chan = p->chan + channel;
	int ret;
	uint32_t flags, val;

	if (channel >= dma->plat_data.channels || channel == DMA_CHAN_INVALID) {
		trace_dwdma_error("dw-dma: %d invalid channel %d",
				  dma->plat_data.id, channel);
		return -EINVAL;
	}
	ret = 0;
	trace_dwdma("dw-dma: %d channel %d stop", dma->plat_data.id, channel);

	spin_lock_irq(&dma->lock, flags);
	val = edma_read(dma, EDMA_CH_CSR);
	val &= ~EDMA_CH_CSR_ERQ;
	emda_write(dma, EDMA_CH_CSR, val);

	spin_unlock_irq(&dma->lock, flags);
	return ret;
}

/* fill in "status" with current DMA channel state and position */
static int dw_dma_status(struct dma *dma, int channel,
			 struct dma_chan_status *status,
			 uint8_t direction)
{
	struct dma_pdata *p = dma_get_drvdata(dma);

	return 0;
	if (channel >= dma->plat_data.channels) {
		trace_dwdma_error("dw-dma: %d invalid channel %d",
				  dma->plat_data.id, channel);
		return -EINVAL;
	}

	status->state = p->chan[channel].status;
	status->r_pos = dw_read(dma, DW_SAR(channel));
	status->w_pos = dw_read(dma, DW_DAR(channel));
	status->timestamp = timer_get_system(platform_timer);

	return 0;
}

/*
 * use array to get burst_elems for specific slot number setting.
 * the relation between msize and burst_elems should be
 * 2 ^ msize = burst_elems
 */
static const uint32_t burst_elems[] = {1, 2, 4, 8};

/* set the DMA channel configuration, source/target address, buffer sizes */
static int edma_set_config(struct dma *dma, int channel,
			     struct dma_sg_config *config)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *chan = p->chan + channel;
	struct dma_sg_elem *sg_elem;
	struct nxp_edma_hw_tcd *tcd,
	uint32_t flags;
	int i, ret = 0;

	if (channel >= dma->plat_data.channels) {
		trace_dwdma_error("dw-dma: %d invalid channel %d",
				  dma->plat_data.id, channel);
		return -EINVAL;
	}

	__dsp_printf("edma: %d channel %d config\n", dma->plat_data.id,
		     channel);
	tcd = rzalloc(RZONE_SYS_RUNTIME,
		      SOF_MEM_CAPS_RAM | SOF_MEM_CAPS_DMA,
		      sizeof(struct nxp_edma_hw_tcd) *
		      chan->desc_count);


	spin_lock_irq(&dma->lock, flags);

	/* default channel config */
	chan->direction = config->direction;
	chan->timer_delay = config->timer_delay;

	if (!config->elem_array.count) {
		__dsp_printf("edma: %d channel %d no elems",
				  dma->plat_data.id, channel);
		ret = -EINVAL;
		goto out;
	}
//	edma_init(dma, DMA_MEM_TO_DEV, tcd, 
out:
	spin_unlock_irq(&dma->lock, flags);
	return ret;
}

/* restore DMA conext after leaving D3 */
static int edma_pm_context_restore(struct dma *dma)
{
	return 0;
}

/* store DMA conext after leaving D3 */
static int edma_pm_context_store(struct dma *dma)
{
	/* disable the DMA controller */
	return 0;
}

static int edma_set_cb(struct dma *dma, int channel, int type,
		void (*cb)(void *data, uint32_t type, struct dma_sg_elem *next),
		void *data)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *chan = p->chan + channel;
	uint32_t flags;

	spin_lock_irq(&dma->lock, flags);
	chan->cb = cb;
	chan->cb_data = data;
	chan->cb_type = type;
	spin_unlock_irq(&dma->lock, flags);

	return 0;
}

/* reload using LLI data */
static inline void dw_dma_chan_reload_lli(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *chan = p->chan + channel;
	struct dw_lli2 *lli = chan->lli_current;

	/* only need to reload if this is a block transfer */
	if (!lli || lli->llp == 0) {
		chan->status = COMP_STATE_PREPARE;
		return;
	}

	/* get current and next block pointers */
	lli = (struct dw_lli2 *)lli->llp;
	chan->lli_current = lli;

	/* channel needs started from scratch, so write SARn, DARn */
	dw_write(dma, DW_SAR(channel), lli->sar);
	dw_write(dma, DW_DAR(channel), lli->dar);

	/* program CTLn */
	dw_write(dma, DW_CTRL_LOW(channel), lli->ctrl_lo);
	dw_write(dma, DW_CTRL_HIGH(channel), lli->ctrl_hi);

	/* program CFGn */
	dw_write(dma, DW_CFG_LOW(channel), chan->cfg_lo);
	dw_write(dma, DW_CFG_HIGH(channel), chan->cfg_hi);

	/* enable the channel */
	dw_write(dma, DW_DMA_CHAN_EN, CHAN_ENABLE(channel));
}

/* reload using callback data */
static inline void dw_dma_chan_reload_next(struct dma *dma, int channel,
					   struct dma_sg_elem *next)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *chan = p->chan + channel;
	struct dw_lli2 *lli = chan->lli_current;

	/* channel needs started from scratch, so write SARn, DARn */
	dw_write(dma, DW_SAR(channel), next->src);
	dw_write(dma, DW_DAR(channel), next->dest);

	/* set transfer size of element */
#if defined CONFIG_BAYTRAIL || defined CONFIG_CHERRYTRAIL \
	|| defined CONFIG_APOLLOLAKE || defined CONFIG_CANNONLAKE \
	|| defined CONFIG_ICELAKE || defined CONFIG_SUECREEK
		lli->ctrl_hi = DW_CTLH_CLASS(p->class) |
			(next->size & DW_CTLH_BLOCK_TS_MASK);
#elif defined CONFIG_BROADWELL || defined CONFIG_HASWELL
	/* for the unit is transaction--TR_WIDTH. */
	lli->ctrl_hi = (next->size / (1 << (lli->ctrl_lo >> 4 & 0x7)))
		& DW_CTLH_BLOCK_TS_MASK;
#endif

	/* program CTLn */
	dw_write(dma, DW_CTRL_LOW(channel), lli->ctrl_lo);
	dw_write(dma, DW_CTRL_HIGH(channel), lli->ctrl_hi);

	/* program CFGn */
	dw_write(dma, DW_CFG_LOW(channel), chan->cfg_lo);
	dw_write(dma, DW_CFG_HIGH(channel), chan->cfg_hi);

	/* enable the channel */
	dw_write(dma, DW_DMA_CHAN_EN, CHAN_ENABLE(channel));
}

static void dw_dma_process_block(struct dma_chan_data *chan,
				 struct dma_sg_elem *next)
{
	/* reload lli by default */
	next->src = DMA_RELOAD_LLI;
	next->dest = DMA_RELOAD_LLI;
	next->size = DMA_RELOAD_LLI;

	if (chan->cb)
		chan->cb(chan->cb_data, DMA_IRQ_TYPE_BLOCK, next);

	if (next->size == DMA_RELOAD_END) {
		tracev_dwdma("dw-dma: %d channel %d block end",
			     chan->id.dma->plat_data.id, chan->id.channel);

		/* disable channel, finished */
		dw_write(chan->id.dma, DW_DMA_CHAN_EN,
			 CHAN_DISABLE(chan->id.channel));
		chan->status = COMP_STATE_PREPARE;
	}

	chan->lli_current->ctrl_hi &= ~DW_CTLH_DONE(1);
	dcache_writeback_region(chan->lli_current, sizeof(*chan->lli_current));

	chan->lli_current = (struct dw_lli2 *)chan->lli_current->llp;
}

static uint64_t dw_dma_work(void *data, uint64_t delay)
{
	struct dma_id *dma_id = data;
	struct dma *dma = dma_id->dma;
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_sg_elem next;
	int i = dma_id->channel;

	tracev_dwdma("dw-dma: %d channel work", dma->plat_data.id,
		     dma_id->channel);

	if (p->chan[i].status != COMP_STATE_ACTIVE) {
		trace_dwdma_error("dw-dma: %d channel %d not running",
				  dma->plat_data.id, dma_id->channel);
		/* skip if channel is not running */
		return 0;
	}

	dw_dma_process_block(&p->chan[i], &next);

	return next.size == DMA_RELOAD_END ? 0 : p->chan[i].timer_delay;
}

#if CONFIG_APOLLOLAKE
/* interrupt handler for DW DMA */
static void dw_dma_irq_handler(void *data)
{
	struct dma_id *dma_id = data;
	struct dma *dma = dma_id->dma;
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_sg_elem next;
	uint32_t status_tfr, status_block, status_err, status_intr;
	uint32_t mask;
	int i = dma_id->channel;

	status_intr = dw_read(dma, DW_INTR_STATUS);
	if (!status_intr) {
		trace_dwdma_error("dw-dma: %d IRQ with no status",
				  dma->plat_data.id);
	}

	tracev_dwdma("dw-dma: %d IRQ status 0x%x", dma->plat_data.id,
		     status_intr);

	/* get the source of our IRQ. */
	status_block = dw_read(dma, DW_STATUS_BLOCK);
	status_tfr = dw_read(dma, DW_STATUS_TFR);

	/* TODO: handle errors, just clear them atm */
	status_err = dw_read(dma, DW_STATUS_ERR);
	if (status_err) {
		trace_dwdma("dw-dma: %d IRQ error 0x%", dma->plat_data.id,
			    status_err);
		dw_write(dma, DW_CLEAR_ERR, status_err & i);
	}

	mask = 0x1 << i;

	/* clear interrupts for channel*/
	dw_write(dma, DW_CLEAR_BLOCK, status_block & mask);
	dw_write(dma, DW_CLEAR_TFR, status_tfr & mask);

	/* skip if channel is not running */
	if (p->chan[i].status != COMP_STATE_ACTIVE) {
		trace_dwdma_error("dw-dma: %d channel %d not running",
				  dma->plat_data.id, dma_id->channel);
		return;
	}

	/* end of a LLI block */
	if (status_block & mask &&
	    p->chan[i].cb_type & DMA_IRQ_TYPE_BLOCK)
		dw_dma_process_block(&p->chan[i], &next);
}

static inline int dw_dma_interrupt_register(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	uint32_t irq = dma_irq(dma, cpu_get_id()) +
		(channel << SOF_IRQ_BIT_SHIFT);
	int ret;

	trace_event(TRACE_CLASS_DMA, "dw_dma_interrupt_register()");

	ret = interrupt_register(irq, IRQ_AUTO_UNMASK, dw_dma_irq_handler,
				 &p->chan[channel].id);
	if (ret < 0) {
		trace_dwdma_error("DWDMA failed to allocate IRQ");
		return ret;
	}

	interrupt_enable(irq);
	return 0;
}

static inline void dw_dma_interrupt_unregister(struct dma *dma, int channel)
{
	uint32_t irq = dma_irq(dma, cpu_get_id()) +
		(channel << SOF_IRQ_BIT_SHIFT);

	interrupt_disable(irq);
	interrupt_unregister(irq);
}
#else
static void dw_dma_process_transfer(struct dma_chan_data *chan,
				    struct dma_sg_elem *next)
{
	/* reload lli by default */
	next->src = DMA_RELOAD_LLI;
	next->dest = DMA_RELOAD_LLI;
	next->size = DMA_RELOAD_LLI;

	if (chan->cb)
		chan->cb(chan->cb_data, DMA_IRQ_TYPE_LLIST, next);

	/* check for reload channel:
	 * next.size is DMA_RELOAD_END, stop this dma copy;
	 * next.size > 0 but not DMA_RELOAD_LLI, use next
	 * element for next copy;
	 * if we are waiting for pause, pause it;
	 * otherwise, reload lli
	 */
	switch (next->size) {
	case DMA_RELOAD_END:
		chan->status = COMP_STATE_PREPARE;
		chan->lli_current = (struct dw_lli2 *)chan->lli_current->llp;
		break;
	case DMA_RELOAD_LLI:
		dw_dma_chan_reload_lli(chan->id.dma, chan->id.channel);
		break;
	default:
		dw_dma_chan_reload_next(chan->id.dma, chan->id.channel, next);
		break;
	}
}

/* interrupt handler for DMA */
static void dw_dma_irq_handler(void *data)
{
	struct dma *dma = data;
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_sg_elem next;
	uint32_t status_tfr;
	uint32_t status_block;
	uint32_t status_block_new;
	uint32_t status_err;
	uint32_t status_intr;
	uint32_t mask;
	uint32_t pmask;
	int i;

	status_intr = dw_read(dma, DW_INTR_STATUS);
	if (!status_intr)
		return;

	tracev_dwdma("dw-dma: %d IRQ status 0x%x", dma->plat_data.id,
		     status_intr);

	/* get the source of our IRQ. */
	status_block = dw_read(dma, DW_STATUS_BLOCK);
	status_tfr = dw_read(dma, DW_STATUS_TFR);

	/* clear interrupts */
	dw_write(dma, DW_CLEAR_BLOCK, status_block);
	dw_write(dma, DW_CLEAR_TFR, status_tfr);

	/* TODO: handle errors, just clear them atm */
	status_err = dw_read(dma, DW_STATUS_ERR);
	dw_write(dma, DW_CLEAR_ERR, status_err);
	if (status_err)
		trace_dwdma_error("dw-dma: %d error 0x%x", dma->plat_data.id,
				  status_err);

	/* clear platform and DSP interrupt */
	pmask = status_block | status_tfr | status_err;
	platform_interrupt_clear(dma_irq(dma, cpu_get_id()), pmask);

	/* confirm IRQ cleared */
	status_block_new = dw_read(dma, DW_STATUS_BLOCK);
	if (status_block_new) {
		trace_dwdma_error("dw-dma: %d status block 0x%x not cleared",
				  dma->plat_data.id, status_block_new);
	}

	for (i = 0; i < dma->plat_data.channels; i++) {

		/* skip if channel is not running */
		if (p->chan[i].status != COMP_STATE_ACTIVE)
			continue;

		mask = 0x1 << i;

#if DW_USE_HW_LLI
		/* end of a LLI block */
		if (status_block & mask &&
		    p->chan[i].cb_type & DMA_IRQ_TYPE_BLOCK)
			dw_dma_process_block(&p->chan[i], &next);
#endif
		/* end of a transfer */
		if (status_tfr & mask &&
		    p->chan[i].cb_type & DMA_IRQ_TYPE_LLIST)
			dw_dma_process_transfer(&p->chan[i], &next);
	}
}

static inline int dw_dma_interrupt_register(struct dma *dma, int channel)
{
	uint32_t irq = dma_irq(dma, cpu_get_id());
	int ret;

	if (!dma->mask_irq_channels) {
		ret = interrupt_register(irq, IRQ_AUTO_UNMASK,
					 dw_dma_irq_handler, dma);
		if (ret < 0) {
			trace_dwdma_error("DWDMA failed to allocate IRQ");
			return ret;
		}

		interrupt_enable(irq);
	}

	dma->mask_irq_channels = dma->mask_irq_channels | BIT(channel);

	return 0;
}

static inline void dw_dma_interrupt_unregister(struct dma *dma, int channel)
{
	uint32_t irq = dma_irq(dma, cpu_get_id());

	dma->mask_irq_channels = dma->mask_irq_channels & ~BIT(channel);

	if (!dma->mask_irq_channels) {
		interrupt_disable(irq);
		interrupt_unregister(irq);
	}
}
#endif

static int edma_probe(struct dma *dma)
{
	struct dma_pdata *edma_pdata;
	int i, ret;

	__dsp_printf("edma probe, id %d\n", dma->plat_data.id);

	if (dma_get_drvdata(dma))
		return -EEXIST; /* already created */

	/* allocate private data */
	edma_pdata = rzalloc(RZONE_SYS_RUNTIME | RZONE_FLAG_UNCACHED,
			   SOF_MEM_CAPS_RAM, sizeof(*edma_pdata));
	if (!edma_pdata) {
		trace_error(TRACE_CLASS_DMA, "dw_dma_probe() error: "
			    "alloc failed");
		return -ENOMEM;
	}
	dma_set_drvdata(dma, edma_pdata);

	spinlock_init(&dma->lock);
#if 0
	ret = edma_init(dma);
	if (ret < 0)
		return ret;
#endif
	/* init work */
	for (i = 0; i < dma->plat_data.channels; i++) {
		dw_pdata->chan[i].id.dma = dma;
		dw_pdata->chan[i].id.channel = i;
		dw_pdata->chan[i].status = COMP_STATE_INIT;
	}

	/* init number of channels draining */
	atomic_init(&dma->num_channels_busy, 0);

	return 0;
}

static int edma_remove(struct dma *dma)
{
	__dsp_printf("edma_remove() id = %u", dma->plat_data.id);

	rfree(dma_get_drvdata(dma));
	dma_set_drvdata(dma, NULL);

	return 0;
}

const struct dma_ops edma_ops = {
	.channel_get	= edma_channel_get,
	.channel_put	= edma_channel_put,
	.start		= edma_start,
	.stop		= edma_stop,
	.pause		= edma_pause,
	.release	= edma_release,
	.status		= edma_status,
	.set_config	= edma_set_config,
	.set_cb		= edma_set_cb,
	.pm_context_restore	= edma_pm_context_restore,
	.pm_context_store	= edma_pm_context_store,
	.probe		= edma_probe,
	.remove		= edma_remove,
};
