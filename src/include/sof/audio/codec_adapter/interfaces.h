/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2020 Intel Corporation. All rights reserved.
 *
 *
 * \file interfaces.h
 * \brief Description of supported codecs
 * \author Marcin Rajwa <marcin.rajwa@linux.intel.com>
 *
 */
#ifndef __SOF_AUDIO_CODEC_INTERFACES__
#define __SOF_AUDIO_CODEC_INTERFACES__

#if CONFIG_CADENCE_CODEC
#include <sof/audio/codec_adapter/codec/cadence.h>
#endif /* CONFIG_CADENCE_CODEC */

#if CONFIG_WAVES_CODEC
#include <sof/audio/codec_adapter/codec/waves.h>
#endif

#define CADENCE_ID 0xCADE01
#define WAVES_ID   0x574101

/*****************************************************************************/
/* Linked codecs interfaces						     */
/*****************************************************************************/
#endif /* __SOF_AUDIO_CODEC_INTERFACES__ */
