#
# Topology for generic Apollolake UP^2 with pcm512x codec and HDMI.
#

# Include topology builder
include(`utils.m4')
include(`dai.m4')
include(`pipeline.m4')
include(`esai.m4')

# Include TLV library
include(`common/tlv.m4')

# Include Token library
include(`sof/tokens.m4')

#
# Define the pipelines
#
# PCM0 <---> volume <----> SSP5 (pcm512x)
# PCM1 ----> volume -----> iDisp1
# PCM2 ----> volume -----> iDisp2
# PCM3 ----> volume -----> iDisp3
# PCM4 ----> volume -----> Media Playback 4
# PCM5 <------------------ DMIC0 (DMIC)
# PCM6 <------------------ DMIC1 (DMIC16kHz)
#

dnl PIPELINE_PCM_ADD(pipeline,
dnl     pipe id, pcm, max channels, format,
dnl     period, priority, core,
dnl     pcm_min_rate, pcm_max_rate, pipeline_rate,
dnl     time_domain, sched_comp)

# Low Latency playback pipeline 1 on PCM 0 using max 2 channels of s32le.
# Set 1000us deadline on core 0 with priority 0
PIPELINE_PCM_ADD(sof/pipe-low-latency-playback.m4,
	1, 0, 2, s32le,
	1000, 0, 0,
	48000, 48000, 48000)

#
# DAIs configuration
#

dnl DAI_ADD(pipeline,
dnl     pipe id, dai type, dai_index, dai_be,
dnl     buffer, periods, format,
dnl     deadline, priority, core, time_domain)

# playback DAI is SSP5 using 2 periods
# Buffers use s24le format, 1000us deadline on core 0 with priority 0
DAI_ADD(sof/pipe-dai-playback.m4,
	1, ESAI, 0, esai0-cs42888,
	PIPELINE_SOURCE_1, 2, s24le,
	1000, 0, 0, SCHEDULE_TIME_DOMAIN_DMA)

# capture DAI is SSP5 using 2 periods
# Buffers use s16le format, 1000us deadline on core 0 with priority 0
DAI_ADD(sof/pipe-dai-capture.m4,
	6, ESAI, 0, esai0-cs42888,
	PIPELINE_SINK_6, 2, s24le,
	1000, 0, 0, SCHEDULE_TIME_DOMAIN_DMA)

# Media playback pipeline 5 on PCM 4 using max 2 channels of s16le.
# Set 1000us deadline on core 0 with priority 0
PIPELINE_PCM_ADD(sof/pipe-pcm-media.m4,
	5, 4, 2, s16le,
	1000, 0, 0,
	8000, 96000, 48000,
	SCHEDULE_TIME_DOMAIN,
	PIPELINE_PLAYBACK_SCHED_COMP_1)

# Connect pipelines together
SectionGraph."media-pipeline" {
	index "0"

	lines [
		dapm(PIPELINE_MIXER_1, PIPELINE_SOURCE_5)
	]
}

# PCM Low Latency, id 0
dnl PCM_PLAYBACK_ADD(name, pcm_id, playback)
PCM_DUPLEX_ADD(Port0, 0, PIPELINE_PCM_1, PIPELINE_PCM_6)

#
# BE configurations - overrides config in ACPI if present
#

dnl DAI_CONFIG(type, dai_index, link_id, name, ssp_config/dmic_config)
#SSP 5 (ID: 0)
DAI_CONFIG(ESAI, 0, 0, esai0-cs42888,
	ESAI_CONFIG(I2S, ESAI_CLOCK(mclk, 49152000,, codec_mclk_in),
		ESAI_CLOCK(bclk, 3072000, codec_slave),
		ESAI_CLOCK(fsync, 48000, codec_slave),
		ESAI_TDM(2, 32, 3, 3),
		ESAI_CONFIG_DATA(ESAI, 0, 0)))
