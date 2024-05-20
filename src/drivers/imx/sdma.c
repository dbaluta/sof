// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright 2020 NXP
//
// Author: Paul Olaru <paul.olaru@nxp.com>

#include <sof/audio/component.h>
#include <sof/drivers/sdma.h>
#include <rtos/timer.h>
#include <rtos/alloc.h>
#include <sof/lib/dma.h>
#include <sof/lib/io.h>
#include <sof/lib/notifier.h>
#include <sof/lib/uuid.h>
#include <rtos/wait.h>
#include <sof/platform.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

LOG_MODULE_REGISTER(sdma, CONFIG_SOF_LOG_LEVEL);

/* 70d223ef-2b91-4aac-b444-d89a0db2793a */
DECLARE_SOF_UUID("sdma", sdma_uuid, 0x70d223ef, 0x2b91, 0x4aac,
		 0xb4, 0x44, 0xd8, 0x9a, 0x0d, 0xb2, 0x79, 0x3a);

DECLARE_TR_CTX(sdma_tr, SOF_UUID(sdma_uuid), LOG_LEVEL_INFO);

#define SDMA_BUFFER_PERIOD_COUNT 2

#if 0
void do_log(char const *p)
{
	tr_info(&sdma_tr, p);
}

#endif
struct sdma_bd {
	/* SDMA BD (buffer descriptor) configuration */
	uint32_t config;

	/* Buffer addresses, typically source and destination in some
	 * order, dependent on script
	 */
	uint32_t buf_addr;
	uint32_t buf_xaddr;
} __packed;

/* SDMA core context */
struct sdma_context {
	uint32_t pc;
	uint32_t spc;
	uint32_t g_reg[8];
	uint32_t dma_xfer_regs[14];
	uint32_t scratch[8];
} __packed;

/* SDMA channel control block */
struct sdma_ccb {
	uint32_t current_bd_paddr;
	uint32_t base_bd_paddr;
	uint32_t status;
	uint32_t reserved; /* No channel descriptor implemented */
};

/* This structure includes all SDMA related channel data */
struct sdma_chan {
	/* Statically allocate BDs; we can change if we ever need dynamic
	 * allocation
	 */
	struct sdma_bd desc[SDMA_MAX_BDS];
	int desc_count;
	struct sdma_context *ctx;
	struct sdma_ccb *ccb;
	int hw_event;
	int next_bd;
	int sdma_chan_type;
	int fifo_paddr;

	unsigned int watermark_level;
	unsigned int sw_done_sel; /* software done selector */
};

/* Private data for the whole controller */
struct sdma_pdata {
	/* Statically allocate channel private data and contexts array.
	 * CCBs must be allocated as array anyway.
	 */
	struct sdma_chan *chan_pdata;
	struct sdma_context *contexts;
	struct sdma_ccb *ccb_array;
};

static void sdma_set_overrides(struct dma_chan_data *channel,
			       bool event_override, bool host_override)
{
	tr_dbg(&sdma_tr, "sdma_set_overrides(%d, %d)", event_override,
	       host_override);
	dma_reg_update_bits(channel->dma, SDMA_EVTOVR, BIT(channel->index),
			    event_override ? BIT(channel->index) : 0);
	dma_reg_update_bits(channel->dma, SDMA_HOSTOVR, BIT(channel->index),
			    host_override ? BIT(channel->index) : 0);
}

#if 0
static void sdma_enable_channel(struct dma *dma, int channel)
{
	dma_reg_write(dma, SDMA_HSTART, BIT(channel));
}
#endif

#if 0
static void sdma_disable_channel(struct dma *dma, int channel)
{
	dma_reg_write(dma, SDMA_STOP_STAT, BIT(channel));
}

#endif
#if 0
static int sdma_run_c0(struct dma *dma, uint8_t cmd, uint32_t buf_addr,
		       uint16_t sdma_addr, uint16_t count)
{
	struct dma_chan_data *c0 = dma->chan;
	struct sdma_chan *c0data = dma_chan_get_data(c0);
	int ret;

	tr_dbg(&sdma_tr, "sdma_run_c0 cmd %d buf_addr 0x%08x sdma_addr 0x%04x count %d",
	       cmd, buf_addr, sdma_addr, count);

	c0data->desc[0].config = SDMA_BD_CMD(cmd) | SDMA_BD_COUNT(count)
		| SDMA_BD_WRAP | SDMA_BD_DONE;
	c0data->desc[0].buf_addr = buf_addr;
	c0data->desc[0].buf_xaddr = sdma_addr;
	if (sdma_addr)
		c0data->desc[0].config |= SDMA_BD_EXTD;

	c0data->ccb->current_bd_paddr = (uint32_t)&c0data->desc[0];
	c0data->ccb->base_bd_paddr = (uint32_t)&c0data->desc[0];

	/* Writeback descriptors and CCB */
	dcache_writeback_region(c0data->desc,
				sizeof(c0data->desc[0]));
	dcache_writeback_region(c0data->ccb, sizeof(*c0data->ccb));

	/* Set event override to true so we can manually start channel */
	sdma_set_overrides(c0, true, false);

	sdma_enable_channel(dma, 0);

	/* 1 is BIT(0) for channel 0, the bit will be cleared as the
	 * channel finishes execution. 1ms is sufficient if everything is fine.
	 */
	ret = poll_for_register_delay(dma_base(dma) + SDMA_STOP_STAT,
				      1, 0, 1000);
	if (ret >= 0)
		ret = 0;

	if (ret < 0)
		tr_err(&sdma_tr, "SDMA channel 0 timed out");

	/* Switch to dynamic context switch mode if needed. This saves power. */
	if ((dma_reg_read(dma, SDMA_CONFIG) & SDMA_CONFIG_CSM_MSK) ==
	    SDMA_CONFIG_CSM_STATIC)
		dma_reg_update_bits(dma, SDMA_CONFIG, SDMA_CONFIG_CSM_MSK,
				    SDMA_CONFIG_CSM_DYN);

	tr_dbg(&sdma_tr, "sdma_run_c0 done, ret = %d", ret);

	return ret;
}
#endif

#if 0
static int sdma_register_init(struct dma *dma)
{
	int ret;
	struct sdma_pdata *pdata = dma_get_drvdata(dma);
	int i;

	tr_dbg(&sdma_tr, "sdma_register_init");
	dma_reg_write(dma, SDMA_RESET, 1);
	/* Wait for 10us */
	ret = poll_for_register_delay(dma_base(dma) + SDMA_RESET, 1, 0, 1000);
	if (ret < 0) {
		tr_err(&sdma_tr, "SDMA reset error, base address %p",
		       (void *)dma_base(dma));
		return ret;
	}

	dma_reg_write(dma, SDMA_MC0PTR, 0);

	/* Ack all interrupts, they're leftover */
	dma_reg_write(dma, SDMA_INTR, MASK(31, 0));

	/* SDMA requires static context switch for first execution of channel 0
	 * in the future. Set it to static here, then have it change to dynamic
	 * after this first execution of channel 0 completes.
	 *
	 * Also set ACR bit according to hardware configuration. Each platform
	 * may have a different configuration.
	 */
#if SDMA_CORE_RATIO
	dma_reg_update_bits(dma, SDMA_CONFIG,
			    SDMA_CONFIG_CSM_MSK | SDMA_CONFIG_ACR,
			    SDMA_CONFIG_ACR);
#else
	dma_reg_update_bits(dma, SDMA_CONFIG,
			    SDMA_CONFIG_CSM_MSK | SDMA_CONFIG_ACR, 0);
#endif
	/* Set 32-word scratch memory size */
	dma_reg_update_bits(dma, SDMA_CHN0ADDR, BIT(14), BIT(14));

	/* Reset channel enable map (it doesn't reset with the controller).
	 * It shall be updated whenever channels need to be activated by
	 * hardware events.
	 */
	for (i = 0; i < SDMA_HWEVENTS_COUNT; i++)
		dma_reg_write(dma, SDMA_CHNENBL(i), 0);

	for (i = 0; i < dma->plat_data.channels; i++)
		dma_reg_write(dma, SDMA_CHNPRI(i), 0);

	/* Write ccb_array pointer to SDMA controller */
	dma_reg_write(dma, SDMA_MC0PTR, (uint32_t)pdata->ccb_array);

	return 0;
}

static void sdma_init_c0(struct dma *dma)
{
	struct dma_chan_data *c0 = &dma->chan[0];
	struct sdma_pdata *sdma_pdata = dma_get_drvdata(dma);
	struct sdma_chan *pdata = &sdma_pdata->chan_pdata[0];

	tr_dbg(&sdma_tr, "sdma_init_c0");
	c0->status = COMP_STATE_READY;

	/* Reset channel 0 private data */
	memset(pdata, 0, sizeof(*pdata));
	pdata->ctx = sdma_pdata->contexts;
	pdata->ccb = sdma_pdata->ccb_array;
	pdata->hw_event = -1;
	dma_chan_set_data(c0, pdata);

	dma_reg_write(dma, SDMA_CHNPRI(0), SDMA_MAXPRI);
}
#endif
#if 0
static int sdma_boot(struct dma *dma)
{
	int ret;

	tr_dbg(&sdma_tr, "sdma_boot");
#if 1
	ret = sdma_register_init(dma);
	if (ret < 0)
		return ret;
#endif
	sdma_init_c0(dma);

	tr_dbg(&sdma_tr, "sdma_boot done");
	return 0;
}
#endif
#if 0
static int sdma_upload_context(struct dma_chan_data *chan)
{
	struct sdma_chan *pdata = dma_chan_get_data(chan);

	/* Ensure context is ready for upload */
	dcache_writeback_region(pdata->ctx, sizeof(*pdata->ctx));

	tr_dbg(&sdma_tr, "sdma_upload_context for channel %d", chan->index);

	/* Last parameters are unneeded for this command and are ignored;
	 * set to 0.
	 */
	return sdma_run_c0(chan->dma, SDMA_CMD_C0_SET_DM, (uint32_t)pdata->ctx,
			   SDMA_SRAM_CONTEXTS_BASE +
				 /* https://trac.cppcheck.net/ticket/10179 */
				 /* cppcheck-suppress divideSizeof */
			   chan->index * sizeof(*pdata->ctx) / 4,
				 /* https://trac.cppcheck.net/ticket/10179 */
				 /* cppcheck-suppress divideSizeof */
			   sizeof(*pdata->ctx) / 4);
}

#if CONFIG_HAVE_SDMA_FIRMWARE
static int sdma_load_firmware(struct dma *dma, void *buf, int addr, int size)
{
	return sdma_run_c0(dma->chan->dma, SDMA_CMD_C0_SET_PM,
			   (uint32_t)buf, addr, size / 2);
}
#endif
#endif
/* Below SOF related functions will be placed */

static int sdma_probe(struct dma *dma)
{
	//int channel;
	//int ret;
	//struct sdma_pdata *pdata;

	LOG_INF("SDMA probe .. dma chans\n");

	if (dma->chan) {
		tr_err(&sdma_tr, "SDMA: Repeated probe");
		return -EEXIST;
	}

	tr_info(&sdma_tr, "SDMA: probe %x", (int)dma_nxp_sdma_get_base(dma->z_dev));
#if 0
	dma->chan = rzalloc(SOF_MEM_ZONE_RUNTIME, 0, SOF_MEM_CAPS_RAM,
			    dma->plat_data.channels *
			    sizeof(struct dma_chan_data));
	if (!dma->chan) {
		tr_err(&sdma_tr, "SDMA: Probe failure, unable to allocate channel descriptors");
		return -ENOMEM;
	}

#endif
	dma_nxp_sdma_init(dma->z_dev);

	return 0;
}

static int sdma_remove(struct dma *dma)
{
	return 0;
}

static struct dma_chan_data *sdma_channel_get(struct dma *dma,
					      unsigned int chan)
{
	struct sdma_pdata *pdata = dma_get_drvdata(dma);
	struct dma_chan_data *channel;
	struct sdma_chan *cdata;
	int i;
	/* Ignoring channel 0; let's just allocate a free channel */

	tr_dbg(&sdma_tr, "sdma_channel_get");
	for (i = 1; i < dma->plat_data.channels; i++) {
		channel = &dma->chan[i];
		if (channel->status != COMP_STATE_INIT)
			continue;

		/* Reset channel private data */
		cdata = &pdata->chan_pdata[i];
		memset(cdata, 0, sizeof(*cdata));
		cdata->ctx = pdata->contexts + i;
		cdata->ccb = pdata->ccb_array + i;
		cdata->hw_event = -1;

		channel->status = COMP_STATE_READY;
		channel->index = i;
		dma_chan_set_data(channel, cdata);

		/* Allow events, allow manual */
		sdma_set_overrides(channel, false, false);
		dma_nxp_sdma_channel_get(dma->z_dev, i);

		tr_info(&sdma_tr, "sdma_channel_get ... %d base %x", i, (int)dma_nxp_sdma_get_base(dma->z_dev));
		return channel;
	}
	tr_err(&sdma_tr, "sdma no channel free");
	return NULL;
}

#if 0
static void sdma_enable_event(struct dma_chan_data *channel, int eventnum)
{
	struct sdma_chan *pdata = dma_chan_get_data(channel);

	tr_dbg(&sdma_tr, "sdma_enable_event(%d, %d)", channel->index, eventnum);

	if (eventnum < 0 || eventnum > SDMA_HWEVENTS_COUNT)
		return; /* No change if request is invalid */

	dma_reg_update_bits(channel->dma, SDMA_CHNENBL(eventnum),
			    BIT(channel->index), BIT(channel->index));

	if (pdata->sw_done_sel & BIT(31)) {
		unsigned int done0;

		done0 = SDMA_DONE0_CONFIG_DONE_SEL | ~SDMA_DONE0_CONFIG_DONE_DIS;
		dma_reg_update_bits(channel->dma, SDMA_DONE0_CONFIG, 0xFF, done0);
	}
}
#endif
static void sdma_disable_event(struct dma_chan_data *channel, int eventnum)
{
	tr_dbg(&sdma_tr, "sdma_disable_event(%d, %d)", channel->index, eventnum);

	if (eventnum < 0 || eventnum > SDMA_HWEVENTS_COUNT)
		return; /* No change if request is invalid */

	dma_reg_update_bits(channel->dma, SDMA_CHNENBL(eventnum),
			    BIT(channel->index), 0);
}

static void sdma_channel_put(struct dma_chan_data *channel)
{
	struct sdma_chan *pdata = dma_chan_get_data(channel);

	if (channel->status == COMP_STATE_INIT)
		return; /* Channel was already free */
	tr_dbg(&sdma_tr, "sdma_channel_put(%d)", channel->index);

	dma_interrupt_legacy(channel, DMA_IRQ_CLEAR);
	sdma_disable_event(channel, pdata->hw_event);
	sdma_set_overrides(channel, false, false);
	channel->status = COMP_STATE_INIT;
}

static int sdma_start(struct dma_chan_data *channel)
{
	tr_info(&sdma_tr, "sdma_start(%d)", channel->index);

	LOG_INF("sdma_start");

	if (channel->status != COMP_STATE_PREPARE &&
	    channel->status != COMP_STATE_PAUSED)
		return -EINVAL;

	channel->status = COMP_STATE_ACTIVE;

	dma_nxp_sdma_start(channel->dma->z_dev, channel->index);

#if 0
	sdma_enable_channel(channel->dma, channel->index);
#endif
	return 0;
}

static int sdma_stop(struct dma_chan_data *channel)
{


	/* do not try to stop multiple times */
	if (channel->status != COMP_STATE_ACTIVE &&
	    channel->status != COMP_STATE_PAUSED)
		return 0;

	channel->status = COMP_STATE_READY;

	tr_info(&sdma_tr, "sdma_stop(%d)", channel->index);

	dma_nxp_sdma_stop(channel->dma->z_dev, channel->index);
#if 0
	sdma_disable_channel(channel->dma, channel->index);
#endif
	return 0;
}

static int sdma_pause(struct dma_chan_data *channel)
{
	struct sdma_chan *pdata = dma_chan_get_data(channel);

	if (channel->status != COMP_STATE_ACTIVE)
		return -EINVAL;

	channel->status = COMP_STATE_PAUSED;

	/* Manually controlled channels need not be paused. */
	if (pdata->hw_event != -1)
		dma_reg_update_bits(channel->dma, SDMA_HOSTOVR,
				    BIT(channel->index), 0);

	return 0;
}

static int sdma_release(struct dma_chan_data *channel)
{
	if (channel->status != COMP_STATE_PAUSED)
		return -EINVAL;

	channel->status = COMP_STATE_PREPARE;

	/* No pointer realignment is necessary for release, context points
	 * correctly to beginning of the following BD.
	 */
	return 0;
}

static int sdma_copy(struct dma_chan_data *channel, int bytes, uint32_t flags)
{
	struct sdma_chan *pdata = dma_chan_get_data(channel);
#if 0
	struct dma_cb_data next = {
		.channel = channel,
		.elem.size = bytes,
	};
#endif
	int idx;

	tr_dbg(&sdma_tr, "sdma_copy");

	idx = (pdata->next_bd + 1) % 2;
	pdata->next_bd = idx;

	//dma_nxp_sdma_callback(channel->dma->z_dev, channel->index, idx);

#if 0
	/* Work around the fact that we cannot allocate uncached memory
	 * on all platforms supporting SDMA.
	 */
	dcache_invalidate_region(&pdata->desc[idx].config,
				 sizeof(pdata->desc[idx].config));
	pdata->desc[idx].config |= SDMA_BD_DONE;
	dcache_writeback_region(&pdata->desc[idx].config,
				sizeof(pdata->desc[idx].config));

	notifier_event(channel, NOTIFIER_ID_DMA_COPY,
		       NOTIFIER_TARGET_CORE_LOCAL, &next, sizeof(next));

	sdma_enable_channel(channel->dma, channel->index);
#endif
	return 0;
}

static int sdma_status(struct dma_chan_data *channel,
		       struct dma_chan_status *status, uint8_t direction)
{

	struct dma_status stat;

	dma_nxp_sdma_get_status(channel->dma->z_dev, channel->index, &stat);

	status->w_pos = stat.write_position;
	status->r_pos = stat.read_position;

	return 0;
}

static struct dma_config *helper_set_dma_config(struct dma_sg_config *config)
{
	//struct dma_sg_config *config = &dd->config;
	struct dma_config *dma_cfg;
	struct dma_block_config *dma_block_cfg;
	struct dma_block_config *prev = NULL;
	int direction = SOF_IPC_STREAM_PLAYBACK;
	int i;
	tr_info(&sdma_tr, "helper_set_dma_config() %x", (int)config);

#if 1
	tr_info(&sdma_tr, "helper_set_dma_config()  %d %d config cnt %d size %d",
		config->src_width, config->dest_width, config->elem_array.count,
		(int)sizeof(*dma_cfg));
#endif
	dma_cfg = rballoc(SOF_MEM_FLAG_COHERENT, SOF_MEM_CAPS_RAM | SOF_MEM_CAPS_DMA,
			  sizeof(struct dma_config));
	if (!dma_cfg) {
		tr_err(&sdma_tr, "dai_set_dma_config(): dma_cfg allocation failed");
		return NULL;
	}

	if (direction == SOF_IPC_STREAM_PLAYBACK)
		dma_cfg->channel_direction = MEMORY_TO_PERIPHERAL;
	else
		dma_cfg->channel_direction = PERIPHERAL_TO_MEMORY;

#if 1
	tr_info(&sdma_tr, "helper_set_dma_config() (2)");
#endif
	dma_cfg->source_data_size = config->src_width;
	dma_cfg->dest_data_size = config->dest_width;

#if 0
	tr_info(&sdma_tr, "helper_set_dma_config() (3)");
#endif
	if (config->burst_elems)
		dma_cfg->source_burst_length = config->burst_elems;
	else
		dma_cfg->source_burst_length = 8;

	dma_cfg->dest_burst_length = dma_cfg->source_burst_length;
	dma_cfg->cyclic = config->cyclic;
	dma_cfg->user_data = NULL;
	dma_cfg->dma_callback = NULL;
	dma_cfg->block_count = config->elem_array.count;
	if (direction == SOF_IPC_STREAM_PLAYBACK)
		dma_cfg->dma_slot = config->dest_dev;
	else
		dma_cfg->dma_slot = config->src_dev;

	tr_info(&sdma_tr, "helper_set_dma_config() blk cnt %d src_width %d dst_width %d",
		(int)dma_cfg->block_count, (int)dma_cfg->source_data_size, (int)dma_cfg->dest_data_size);


	dma_block_cfg = rballoc(SOF_MEM_FLAG_COHERENT, SOF_MEM_CAPS_RAM | SOF_MEM_CAPS_DMA,
				sizeof(struct dma_block_config) * dma_cfg->block_count);
	if (!dma_block_cfg) {
		rfree(dma_cfg);
		tr_err(&sdma_tr, "dai_set_dma_config: dma_block_config allocation failed");
		return NULL;
	}


	tr_info(&sdma_tr, "alloc ok!");

	dma_cfg->head_block = dma_block_cfg;
	for (i = 0; i < dma_cfg->block_count; i++) {
				dma_block_cfg->dest_scatter_en = config->scatter;
		dma_block_cfg->block_size = config->elem_array.elems[i].size;
		dma_block_cfg->source_address = config->elem_array.elems[i].src;
		dma_block_cfg->dest_address = config->elem_array.elems[i].dest;
		if (direction == SOF_IPC_STREAM_PLAYBACK) {
			dma_block_cfg->source_addr_adj = DMA_ADDR_ADJ_DECREMENT;
			dma_block_cfg->dest_addr_adj = DMA_ADDR_ADJ_INCREMENT;
		} else {
			dma_block_cfg->source_addr_adj = DMA_ADDR_ADJ_INCREMENT;
			dma_block_cfg->dest_addr_adj = DMA_ADDR_ADJ_DECREMENT;
		}

		tr_info(&sdma_tr, "LOOP %d scatter %d src_addr %x dst_adrr %x",
			i, config->scatter,  dma_block_cfg->source_address,
			dma_block_cfg->dest_address);


		prev = dma_block_cfg;
		prev->next_block = ++dma_block_cfg;
	}
	if (prev)
		prev->next_block = dma_cfg->head_block;

//	dd->z_config = dma_cfg;


	return dma_cfg;
}

static int sdma_set_config(struct dma_chan_data *channel,
			   struct dma_sg_config *config)
{
	struct dma_config *dma_cfg;
	int ret;

	LOG_ERR("sdma set config\n");
	dma_cfg = helper_set_dma_config(config);

	tr_info(&sdma_tr, "calling into config index %d", channel->index);


	ret = dma_nxp_sdma_config(channel->dma->z_dev, channel->index, dma_cfg);
	tr_info(&sdma_tr, "callint oug of config");
	return ret;
}


#if 0
static int sdma_set_config(struct dma_chan_data *channel,
			   struct dma_sg_config *config)
{
	struct sdma_chan *pdata = dma_chan_get_data(channel);
	int ret;

	tr_dbg(&sdma_tr, "sdma_set_config channel %d", channel->index);

	ret = sdma_read_config(channel, config);
	if (ret < 0)
		return ret;

	channel->is_scheduling_source = config->is_scheduling_source;
	channel->direction = config->direction;

	ret = sdma_prep_desc(channel, config);
	if (ret < 0)
		return ret;

	/* allow events + allow manual start */
	sdma_set_overrides(channel, false, false);

	/* Upload context */
	ret = sdma_upload_context(channel);
	if (ret < 0) {
		tr_err(&sdma_tr, "Unable to upload context, bailing");
		return ret;
	}

	tr_dbg(&sdma_tr, "SDMA context uploaded");
	/* Context uploaded, we can set up events now */
	sdma_enable_event(channel, pdata->hw_event);

	/* Finally set channel priority */
	dma_reg_write(channel->dma, SDMA_CHNPRI(channel->index), SDMA_DEFPRI);

	channel->status = COMP_STATE_PREPARE;

	return 0;

}

#endif

static int sdma_interrupt(struct dma_chan_data *channel, enum dma_irq_cmd cmd)
{
	if (!channel->index)
		return 0;

	switch (cmd) {
	case DMA_IRQ_STATUS_GET:
		/* Any nonzero value means interrupt is active */
		return dma_reg_read(channel->dma, SDMA_INTR) &
			BIT(channel->index);
	case DMA_IRQ_CLEAR:
		dma_reg_write(channel->dma, SDMA_INTR, BIT(channel->index));
		return 0;
	case DMA_IRQ_MASK:
	case DMA_IRQ_UNMASK:
		/* We cannot control interrupts except by resetting channel to
		 * have it reread the buffer descriptors. That cannot be done
		 * in the context where this function is called. Silently ignore
		 * requests to mask/unmask per-channel interrupts.
		 */
		return 0;
	default:
		tr_err(&sdma_tr, "sdma_interrupt unknown cmd %d", cmd);
		return -EINVAL;
	}
}

static int sdma_get_attribute(struct dma *dma, uint32_t type, uint32_t *value)
{
	switch (type) {
	case DMA_ATTR_BUFFER_ALIGNMENT:
	case DMA_ATTR_COPY_ALIGNMENT:
		/* Use a conservative value, because some scripts
		 * require an alignment of 4 while others can read
		 * unaligned data. Account for those which require
		 * aligned data.
		 */
		*value = 4;
		break;
	case DMA_ATTR_BUFFER_ADDRESS_ALIGNMENT:
		*value = PLATFORM_DCACHE_ALIGN;
		break;
	case DMA_ATTR_BUFFER_PERIOD_COUNT:
		*value = SDMA_BUFFER_PERIOD_COUNT;
		break;
	default:
		return -ENOENT; /* Attribute not found */
	}

	return 0;
}

static int sdma_get_data_size(struct dma_chan_data *channel, uint32_t *avail,
			      uint32_t *free)
{
	/* Check buffer descriptors, those with "DONE" = 0 are for the
	 * host, "DONE" = 1 are for SDMA. The host side are either
	 * available or free.
	 */
	struct sdma_chan *pdata = dma_chan_get_data(channel);
	uint32_t result_data = 0;
	int i;

	tr_dbg(&sdma_tr, "sdma_get_data_size(%d)", channel->index);
	if (channel->index == 0) {
		/* Channel 0 shouldn't have this called anyway */
		tr_err(&sdma_tr, "Please do not call get_data_size on SDMA channel 0!");
		*avail = *free = 0;
		return -EINVAL;
	}

	dcache_invalidate_region(pdata->desc, sizeof(pdata->desc[0]) * SDMA_MAX_BDS);

	for (i = 0; i < pdata->desc_count && i < SDMA_MAX_BDS; i++) {
		if (pdata->desc[i].config & SDMA_BD_DONE)
			continue; /* These belong to SDMA controller */
		result_data += pdata->desc[i].config &
			SDMA_BD_COUNT_MASK;
	}

	*avail = *free = 0;
	switch (channel->direction) {
	case DMA_DIR_MEM_TO_DEV:
		*free = result_data;
		break;
	case DMA_DIR_DEV_TO_MEM:
		*avail = result_data;
		break;
	default:
		tr_err(&sdma_tr, "sdma_get_data_size channel invalid direction");
		return -EINVAL;
	}
	return 0;

}

const struct dma_ops sdma_ops = {
	.channel_get	= sdma_channel_get,
	.channel_put	= sdma_channel_put,
	.start		= sdma_start,
	.stop		= sdma_stop,
	.pause		= sdma_pause,
	.release	= sdma_release,
	.copy		= sdma_copy,
	.status		= sdma_status,
	.set_config	= sdma_set_config,
	.probe		= sdma_probe,
	.remove		= sdma_remove,
	.interrupt	= sdma_interrupt,
	.get_attribute	= sdma_get_attribute,
	.get_data_size	= sdma_get_data_size,
};
