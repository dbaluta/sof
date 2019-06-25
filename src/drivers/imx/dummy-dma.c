// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright 2019 NXP
//
// Author: Daniel Baluta <daniel.baluta@nxp.com>

#include <sof/atomic.h>
#include <sof/dma.h>
#include <errno.h>
#include <stdint.h>
#include <sof/string.h>
#include <config.h>
#include <sof/audio/component.h>

#define trace_dummydma(__e, ...) \
	trace_event(TRACE_CLASS_DMA, __e, ##__VA_ARGS__)
#define tracev_dummydma(__e, ...) \
	tracev_event(TRACE_CLASS_DMA, __e, ##__VA_ARGS__)
#define trace_dummydma_error(__e, ...) \
	trace_error(TRACE_CLASS_DMA, __e, ##__VA_ARGS__)

#define DMA_MAX_CHANS 1

struct dma_chan_data {
	struct dma *dma;
	uint32_t index;
	uint32_t direction;
	uint32_t status;

	struct dma_sg_elem_array *elems;
	uint32_t desc_count;
	int sg_elem_curr_idx;
	int sg_elem_progress_bytes; /* TODO this may remain unused */

	void (*cb)(void *data, uint32_t type,
		   struct dma_sg_elem *next);
	void *cb_data;
	int cb_type;
};

struct dma_pdata {
	struct dma *dma;
	int32_t num_channels;
	struct dma_chan_data chan[DMA_MAX_CHANS];
};

/* allocate next free DMA channel */
static int dummy_dma_channel_get(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	uint32_t flags;

	if (channel >= DMA_MAX_CHANS) {
		trace_dummydma_error("dummy-dmac: %d invalid channel %d",
				     dma->plat_data.id, channel);
		return -EINVAL;
	}
	spin_lock_irq(&dma->lock, flags);

	/* use channel if it's free */
	if (p->chan[channel].status == COMP_STATE_INIT) {
		p->chan[channel].status = COMP_STATE_READY;

		atomic_add(&dma->num_channels_busy, 1);

		/* return channel */
		spin_unlock_irq(&dma->lock, flags);
		return channel;
	}
	spin_unlock_irq(&dma->lock, flags);
	trace_dummydma_error("dummy-dmac: %d no free channel %d",
			     dma->plat_data.id, channel);
	return -ENODEV;
}

static void dummy_dma_channel_put_unlocked(struct dma *dma, int channel)
{
	struct dma_pdata *p = dma_get_drvdata(dma);

	if (channel >= DMA_MAX_CHANS) {
		trace_dummydma_error("dummy-dmac: %d invalid channel %d",
				     dma->plat_data.id, channel);
		return;
	}

	/* Reset channel state */
	p->chan[channel].cb = NULL;
	p->chan[channel].cb_type = 0;
	p->chan[channel].cb_data = NULL;

	p->chan[channel].elems = NULL;
	p->chan[channel].desc_count = 0;
	p->chan[channel].sg_elem_curr_idx = 0;

	p->chan[channel].sg_elem_progress_bytes = 0;

	p->chan[channel].status = COMP_STATE_INIT;
	atomic_sub(&dma->num_channels_busy, 1);
}

/* channel must not be running when this is called */
static void dummy_dma_channel_put(struct dma *dma, int channel)
{
	uint32_t flags;

	spin_lock_irq(&dma->lock, flags);
	dummy_dma_channel_put_unlocked(dma, channel);
	spin_unlock_irq(&dma->lock, flags);
}
static int dummy_dma_start(struct dma *dma, int channel)
{
	return 0;
}


static int dummy_dma_release(struct dma *dma, int channel)
{
	return 0;
}

static int dummy_dma_pause(struct dma *dma, int channel)
{
	return 0;
}

static int dummy_dma_stop(struct dma *dma, int channel)
{
	return 0;
}

/* fill in "status" with current DMA channel state and position */
static int dummy_dma_status(struct dma *dma, int channel,
			 struct dma_chan_status *status,
			 uint8_t direction)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *ch = p->chan + channel;

	(void) direction;

	status->state = ch->status;
	status->flags = 0;
	status->w_pos = ch->sg_elem_progress_bytes; /* TODO check */
	status->r_pos = ch->sg_elem_progress_bytes; /* TODO check */
	status->timestamp = 0; /* TODO */
	return 0;
}

/* set the DMA channel configuration, source/target address, buffer sizes */
static int dummy_dma_set_config(struct dma *dma, int channel,
			     struct dma_sg_config *config)
{
	struct dma_pdata *p = dma_get_drvdata(dma);
	//struct dma_sg_elem *sg_elem; /* TODO what is this: check buffer range? */
	//uint32_t buffer_addr = 0; /* TODO what is this */
	//uint32_t period_bytes = 0; /* TODO what is this */
	//uint32_t buffer_bytes = 0; /* TODO what is this */
	uint32_t flags;
	int ret = 0;

	if (channel >= DMA_MAX_CHANS) {
		trace_dummydma_error("dummy-dmac: %d invalid channel %d",
				     dma->plat_data.id, channel);
		return -EINVAL;
	}

	spin_lock_irq(&dma->lock, flags);

	trace_dummydma("dummy-dmac: %d channel %d -> config", dma->plat_data.id,
		       channel);

	if (!config->elem_array.count) {
		trace_dummydma_error("dummy-dmac: %d channel %d no DMA descriptors",
				     dma->plat_data.id, channel);

		ret = -EINVAL;
		goto out;
	}

	p->chan[channel].direction = config->direction;
	p->chan[channel].desc_count = config->elem_array.count;
	p->chan[channel].elems = &config->elem_array;
	p->chan[channel].sg_elem_curr_idx = 0;

	/* TODO remove next line */
	p->chan[channel].sg_elem_progress_bytes = 0;
	/* TODO is this even correct? Elements may be unused.... */
	//p->chan[channel].period_bytes = period_bytes;
	//p->chan[channel].buffer_bytes = buffer_bytes;

	/* Once the configuration is done, transition into PREPARE (TODO
	 * remove comment) */
	p->chan[channel].status = COMP_STATE_PREPARE;
out:
	spin_unlock_irq(&dma->lock, flags);
	return ret;
}

/* restore DMA context after leaving D3 */
static int dummy_dma_pm_context_restore(struct dma *dma)
{
	return 0;
}

/* store DMA context after leaving D3 */
static int dummy_dma_pm_context_store(struct dma *dma)
{
	return 0;
}

static int dummy_dma_set_cb(struct dma *dma, int channel, int type,
		void (*cb)(void *data, uint32_t type, struct dma_sg_elem *next),
		void *data)
{
	struct dma_pdata *dma_pdata = dma_get_drvdata(dma);

	dma_pdata->chan[channel].cb = cb;
	dma_pdata->chan[channel].cb_data = data;
	dma_pdata->chan[channel].cb_type = type;
	return 0;
}

static int dummy_dma_copy(struct dma *dma, int channel, int bytes,
			  uint32_t flags)
{
	/* Perform the actual copying
	 * TODO */
	struct dma_pdata *p = dma_get_drvdata(dma);
	struct dma_chan_data *ch = p->chan + channel;
	uint32_t i;

	for (i = 0; i < ch->desc_count; i++) {
		struct dma_sg_elem el = ch->elems->elems[i];

		memcpy((void *)el.dest, (void *)el.src, el.size);
		ch->sg_elem_progress_bytes += el.size;
	}
	return 0;
}

static int dummy_dma_probe(struct dma *dma)
{
	struct dma_pdata *dma_pdata;
	int i;

	trace_dummydma("dummy-dmac %d -> probe", dma->plat_data.id);

	if (dma_get_drvdata(dma)) {
		trace_dummydma_error("dummy-dmac %d already created!", dma->plat_data.id);
		return -EEXIST; /* already created */
	}

	/* allocate private data */
	dma_pdata = rzalloc(RZONE_SYS_RUNTIME | RZONE_FLAG_UNCACHED,
			    SOF_MEM_CAPS_RAM, sizeof(*dma_pdata));
	if (!dma_pdata) {
		trace_dummydma_error("dummy-dmac %d: Out of memory!", dma->plat_data.id);
		return -ENOMEM;
	}

	dma_set_drvdata(dma, dma_pdata);

	for (i = 0; i < DMA_MAX_CHANS; i++) {
		dma_pdata->chan[i].dma = dma;
		dma_pdata->chan[i].index = i;
		dma_pdata->chan[i].status = COMP_STATE_INIT;
	}

	atomic_init(&dma->num_channels_busy, 0);

	return 0;
}

static int dummy_dma_remove(struct dma *dma)
{
	tracev_dummydma("dummy_dma %d -> remove", dma->plat_data.id);
	rfree(dma_get_drvdata(dma));
	dma_set_drvdata(dma, NULL);
	return 0;
}

static int dummy_dma_get_data_size(struct dma *dma, int channel,
				   uint32_t *avail, uint32_t *free)
{
	return 0;
}

const struct dma_ops dummy_dma_ops = {
	.channel_get	= dummy_dma_channel_get,
	.channel_put	= dummy_dma_channel_put,
	.start		= dummy_dma_start,
	.stop		= dummy_dma_stop,
	.pause		= dummy_dma_pause,
	.release	= dummy_dma_release,
	.copy		= dummy_dma_copy,
	.status		= dummy_dma_status,
	.set_config	= dummy_dma_set_config,
	.set_cb		= dummy_dma_set_cb,
	.pm_context_restore		= dummy_dma_pm_context_restore,
	.pm_context_store		= dummy_dma_pm_context_store,
	.probe		= dummy_dma_probe,
	.remove		= dummy_dma_remove,
	.get_data_size	= dummy_dma_get_data_size,
};
