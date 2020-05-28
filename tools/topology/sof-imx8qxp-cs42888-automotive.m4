`# Topology for generic' PLATFORM `board with' CODEC `on SSP' SSP_NUM
#
# Topology for i.MX8QXP board with wm8960 codec
#
# Aim to make a demo with two PCMs:
#
# 1. Warning sound.
# 2. Streaming music (using equalizer widget).
# 3. Multiplexer for both PCMs with volumn control. Priority of warning sound is higher than music.
#
# graph is:
#
# PCM0 ----> volume ----------------+
#				                    |--low latency mixer ----> volume ---->  ESAI0(cs42888)
# PCM1 ----> volume ----> SRC ------+


# Include topology builder
include(`utils.m4')
include(`dai.m4')
include(`pipeline.m4')
include(`esai.m4')
include(`pcm.m4')
include(`buffer.m4')

# Include TLV library
include(`common/tlv.m4')

# Include Token library
include(`sof/tokens.m4')

# Include DSP configuration
include(`platform/imx/imx8qxp.m4')

DEBUG_START

#
# Define the pipelines
#
# PCM0(Streaming Music)----> volume --------------+
#                                                 |--low latency mixer ----> volume ----> ESAI0(cs42888)
# PCM1(Warning Sound)  ----> volume ----> SRC ----+

dnl PIPELINE_PCM_ADD(pipeline,
dnl     pipe id, pcm, max channels, format,
dnl     period, priority, core,
dnl     pcm_min_rate, pcm_max_rate, pipeline_rate,
dnl     time_domain, sched_comp)

# eq_fir playback pipeline 1 on PCM 0 using max 2 channels of s32le.
# Set 1000us deadline on core 0 with priority 1
# PCM0(Streaming Music)----> volume --------------+
PIPELINE_PCM_ADD(sof/pipe-eq-fir-volume-playback-mix.m4,
	1, 0, 2, s24le,
	1000, 0, 0,
	48000, 48000, 48000)

#
# DAI configuration
#

dnl DAI_ADD(pipeline,
dnl     pipe id, dai type, dai_index, dai_be,
dnl     buffer, periods, format,
dnl     period, priority, core, time_domain)
# playback DAI is ESAI0 using 2 periods
# Buffers use s24le format, with 48 frame per 1000us on core 0 with priority 0
DAI_ADD(sof/pipe-dai-playback.m4,
	1, ESAI, 0, esai0-cs42888,
	PIPELINE_SOURCE_1, 2, s24le,
	1000, 0, 0, SCHEDULE_TIME_DOMAIN_DMA)


dnl PIPELINE_PCM_ADD(pipeline,
dnl     pipe id, pcm, max channels, format,
dnl     period, priority, core,
dnl     pcm_min_rate, pcm_max_rate, pipeline_rate,
dnl     time_domain, sched_comp)
# PCM Media Playback pipeline 2 on PCM 1 using max 2 channels of s32le.
# 1000us deadline on core 0 with priority 0
# PCM1(Warning Sound)  ----> volume ----> SRC ----+
PIPELINE_PCM_ADD(sof/pipe-pcm-media.m4,
	2, 1, 2, s32le,
	1000, 0, 0,
	8000, 48000, 48000,
	SCHEDULE_TIME_DOMAIN_DMA,
	PIPELINE_PLAYBACK_SCHED_COMP_1)

# Connect pipelines together
SectionGraph."PIPE_NAME" {
	index "0"

	lines [
		# media 0
		dapm(PIPELINE_MIXER_1, PIPELINE_SOURCE_2)
	]
}

PCM_PLAYBACK_ADD(Port5, 0, PIPELINE_PCM_1)

dnl DAI_CONFIG(type, idx, link_id, name, esai_config)
DAI_CONFIG(ESAI, 0, 0, esai0-cs42888,
	ESAI_CONFIG(I2S, ESAI_CLOCK(mclk, 49152000, codec_mclk_in),
		ESAI_CLOCK(bclk, 3072000, codec_slave),
		ESAI_CLOCK(fsync, 48000, codec_slave),
		ESAI_TDM(2, 32, 3, 3),
		ESAI_CONFIG_DATA(ESAI, 0, 0)))

DEBUG_END
