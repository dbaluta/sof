// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright 2023 Intel Corporation. All rights reserved.
//
// Author: Baofeng Tian <baofeng.tian@intel.com>

#include <sof/trace/trace.h>
#include <sof/audio/component_ext.h>
#include <ipc4/copier.h>
#include <sof/audio/dai_copier.h>
#include <ipc/dai.h>

LOG_MODULE_DECLARE(copier, CONFIG_SOF_LOG_LEVEL);

static uint32_t bitmask_to_nibble_channel_map(uint8_t bitmask)
{
	int i;
	int channel_count = 0;
	uint32_t nibble_map = 0;

	for (i = 0; i < 8; i++)
		if (bitmask & BIT(i)) {
			nibble_map |= i << (channel_count * 4);
			channel_count++;
		}

	/* absent channel is represented as 0xf nibble */
	nibble_map |= 0xFFFFFFFF << (channel_count * 4);

	return nibble_map;
}

static int copier_set_alh_multi_gtw_channel_map(struct comp_dev *parent_dev,
						const struct ipc4_copier_module_cfg *copier_cfg,
						int index)
{
	struct copier_data *cd = comp_get_drvdata(parent_dev);
	const struct sof_alh_configuration_blob *alh_blob;
	uint8_t chan_bitmask;
	int channels;

	if (!copier_cfg->gtw_cfg.config_length) {
		comp_err(parent_dev, "No ipc4_alh_multi_gtw_cfg found in blob!");
		return -EINVAL;
	}

	/* For ALH multi-gateway case, configuration blob contains struct ipc4_alh_multi_gtw_cfg
	 * with channel map and channels number for each individual gateway.
	 */
	alh_blob = (const struct sof_alh_configuration_blob *)copier_cfg->gtw_cfg.config_data;
	chan_bitmask = alh_blob->alh_cfg.mapping[index].channel_mask;

	channels = popcount(chan_bitmask);
	if (channels < 1 || channels > SOF_IPC_MAX_CHANNELS) {
		comp_err(parent_dev, "Invalid channels mask: 0x%x", chan_bitmask);
		return -EINVAL;
	}

	cd->channels[index] = channels;
	cd->chan_map[index] = bitmask_to_nibble_channel_map(chan_bitmask);

	return 0;
}

static int copier_dai_init(struct comp_dev *parent_dev,
			   const struct comp_driver *drv,
			   struct comp_ipc_config *config,
			   const struct ipc4_copier_module_cfg *copier,
			   struct pipeline *pipeline,
			   struct ipc_config_dai *dai,
			   enum ipc4_gateway_type type,
			   int index, int dai_count)
{
	struct copier_data *cd = comp_get_drvdata(parent_dev);
	struct dai_data *dd;
	int ret;

	if (cd->direction == SOF_IPC_STREAM_PLAYBACK) {
		enum sof_ipc_frame out_frame_fmt, out_valid_fmt;

		audio_stream_fmt_conversion(copier->out_fmt.depth,
					    copier->out_fmt.valid_bit_depth,
					    &out_frame_fmt,
					    &out_valid_fmt,
					    copier->out_fmt.s_type);
		config->frame_fmt = out_frame_fmt;
		pipeline->sink_comp = parent_dev;
		cd->bsource_buffer = true;
	} else {
		enum sof_ipc_frame in_frame_fmt, in_valid_fmt;

		audio_stream_fmt_conversion(copier->base.audio_fmt.depth,
					    copier->base.audio_fmt.valid_bit_depth,
					    &in_frame_fmt, &in_valid_fmt,
					    copier->base.audio_fmt.s_type);
		config->frame_fmt = in_frame_fmt;
		pipeline->source_comp = parent_dev;
	}

	parent_dev->ipc_config.frame_fmt = config->frame_fmt;

	/* save the channel map and count for ALH multi-gateway */
	if (type == ipc4_gtw_alh && is_multi_gateway(copier->gtw_cfg.node_id)) {
		ret = copier_set_alh_multi_gtw_channel_map(parent_dev, copier, index);
		if (ret < 0)
			return ret;
	}

	dd = rzalloc(SOF_MEM_ZONE_RUNTIME_SHARED, 0, SOF_MEM_CAPS_RAM, sizeof(*dd));
	if (!dd)
		return -ENOMEM;

	ret = dai_zephyr_new(dd, parent_dev, dai);
	if (ret < 0)
		goto free_dd;

	pipeline->sched_id = config->id;

	cd->dd[index] = dd;
	ret = comp_dai_config(cd->dd[index], parent_dev, dai, copier);
	if (ret < 0)
		goto e_zephyr_free;

	cd->endpoint_num++;

	return 0;
e_zephyr_free:
	dai_zephyr_free(dd);
free_dd:
	rfree(dd);
	return ret;
}

/* if copier is linked to non-host gateway, it will manage link dma,
 * ssp, dmic or alh. Sof dai component can support this case so copier
 * reuses dai component to support non-host gateway.
 */
int copier_dai_create(struct comp_dev *parent_dev, struct copier_data *cd,
		      struct comp_ipc_config *config,
		      const struct ipc4_copier_module_cfg *copier,
		      struct pipeline *pipeline)
{
	struct sof_uuid id = {0xc2b00d27, 0xffbc, 0x4150, {0xa5, 0x1a, 0x24,
				0x5c, 0x79, 0xc5, 0xe5, 0x4b}};
	int dai_index[IPC4_ALH_MAX_NUMBER_OF_GTW];
	union ipc4_connector_node_id node_id;
	enum ipc4_gateway_type type;
	const struct comp_driver *drv;
	struct ipc_config_dai dai;
	int dai_count;
	int i, ret;

	drv = ipc4_get_drv((uint8_t *)&id);
	if (!drv)
		return -EINVAL;

	config->type = SOF_COMP_DAI;

	memset(&dai, 0, sizeof(dai));
	dai_count = 1;
	node_id = copier->gtw_cfg.node_id;
	dai_index[dai_count - 1] = node_id.f.v_index;
	dai.direction = get_gateway_direction(node_id.f.dma_type);
	dai.is_config_blob = true;
	dai.sampling_frequency = copier->out_fmt.sampling_frequency;
	dai.feature_mask = copier->copier_feature_mask;

	switch (node_id.f.dma_type) {
	case ipc4_hda_link_output_class:
	case ipc4_hda_link_input_class:
		dai.type = SOF_DAI_INTEL_HDA;
		dai.is_config_blob = true;
		type = ipc4_gtw_link;
		break;
	case ipc4_i2s_link_output_class:
	case ipc4_i2s_link_input_class:
		dai_index[dai_count - 1] = (dai_index[dai_count - 1] >> 4) & 0xF;
		dai.type = SOF_DAI_INTEL_SSP;
		dai.is_config_blob = true;
		type = ipc4_gtw_ssp;
		ret = ipc4_find_dma_config(&dai, (uint8_t *)copier->gtw_cfg.config_data,
					   copier->gtw_cfg.config_length * 4);
		if (ret != 0) {
			comp_err(parent_dev, "No ssp dma_config found in blob!");
			return -EINVAL;
		}
		dai.out_fmt = &copier->out_fmt;
		break;
	case ipc4_alh_link_output_class:
	case ipc4_alh_link_input_class:
		dai.type = SOF_DAI_INTEL_ALH;
		dai.is_config_blob = true;
		type = ipc4_gtw_alh;

		/* copier
		 * {
		 *  gtw_cfg
		 *  {
		 *     gtw_node_id;
		 *     config_length;
		 *     config_data
		 *     {
		 *	   count;
		 *	  {
		 *	     node_id;  \\ normal gtw id
		 *	     mask;
		 *	  }  mapping[MAX_ALH_COUNT];
		 *     }
		 *   }
		 * }
		 */
		 /* get gtw node id in config data */
		if (is_multi_gateway(node_id)) {
			if (copier->gtw_cfg.config_length) {
				const struct sof_alh_configuration_blob *alh_blob =
					(const struct sof_alh_configuration_blob *)
						copier->gtw_cfg.config_data;

				dai_count = alh_blob->alh_cfg.count;
				if (dai_count > IPC4_ALH_MAX_NUMBER_OF_GTW || dai_count < 0) {
					comp_err(parent_dev, "Invalid dai_count: %d", dai_count);
					return -EINVAL;
				}
				for (i = 0; i < dai_count; i++)
					dai_index[i] =
					IPC4_ALH_DAI_INDEX(alh_blob->alh_cfg.mapping[i].alh_id);
			} else {
				comp_err(parent_dev, "No ipc4_alh_multi_gtw_cfg found in blob!");
				return -EINVAL;
			}
		} else {
			dai_index[dai_count - 1] = IPC4_ALH_DAI_INDEX(node_id.f.v_index);
		}

		break;
	case ipc4_dmic_link_input_class:
		dai.type = SOF_DAI_INTEL_DMIC;
		dai.is_config_blob = true;
		type = ipc4_gtw_dmic;

		ret = ipc4_find_dma_config(&dai, (uint8_t *)copier->gtw_cfg.config_data,
					   copier->gtw_cfg.config_length * 4);
		if (ret != 0) {
			comp_err(parent_dev, "No dmic dma_config found in blob!");
			return -EINVAL;
		}
		dai.out_fmt = &copier->out_fmt;
		break;
	default:
		return -EINVAL;
	}

	for (i = 0; i < dai_count; i++) {
		dai.dai_index = dai_index[i];
		ret = copier_dai_init(parent_dev, drv, config, copier, pipeline, &dai, type, i,
				      dai_count);
		if (ret) {
			comp_err(parent_dev, "failed to create dai");
			return ret;
		}
	}

	cd->converter[IPC4_COPIER_GATEWAY_PIN] =
			get_converter_func(&copier->base.audio_fmt, &copier->out_fmt, type,
					   IPC4_DIRECTION(dai.direction));
	if (!cd->converter[IPC4_COPIER_GATEWAY_PIN]) {
		comp_err(parent_dev, "failed to get converter type %d, dir %d",
			 type, dai.direction);
		return -EINVAL;
	}

	/* create multi_endpoint_buffer for ALH multi-gateway case */
	if (dai_count > 1) {
		ret = create_endpoint_buffer(parent_dev, cd, config, copier, type, true, 0);
		if (ret < 0)
			return ret;
	}

	return 0;
}