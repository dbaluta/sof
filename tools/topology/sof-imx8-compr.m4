#
# Topology for generic Apollolake UP^2 with pcm512x codec and no HDMI.
#

# Include topology builder
include(`utils.m4')
include(`dai.m4')
include(`pipeline.m4')
include(`sai.m4')

# Include TLV library
include(`common/tlv.m4')

# Include Token library
include(`sof/tokens.m4')

# Include Apollolake DSP configuration
include(`platform/imx/imx8qxp.m4')

DEBUG_START

#
# Define the pipelines
#
# PPROC is for playback pipeline processing, i.e volume, eq-volume, eq-iir, eq-fir etc
# In this file, volume and eq-volume are used. If anything else is used, please update
# pipeline comments below for tracking purpose.
#


# The pipeline naming notation is pipe-PROCESSING-DIRECTION.m4
define(PIPE_PROC_PLAYBACK, `sof/pipe-`PPROC'-playback.m4')


dnl PIPELINE_PCM_ADD(pipeline,
dnl     pipe id, pcm, max channels, format,
dnl     period, priority, core,
dnl     pcm_min_rate, pcm_max_rate, pipeline_rate,
dnl     time_domain, sched_comp)

# Low Latency playback pipeline 1 on PCM 0 using max 2 channels of s32le.
# Set 1000us deadline on core 0 with priority 0
PIPELINE_PCM_ADD(PIPE_PROC_PLAYBACK,
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

# playback DAI is SAI1 using 2 periods
# Buffers use s24le format, 1000us deadline on core 0 with priority 0
DAI_ADD(sof/pipe-dai-playback.m4,
	1, SAI, 1, sai1-wm8960-hifi,
	PIPELINE_SOURCE_1, 2, s24le,
	1000, 0, 0, SCHEDULE_TIME_DOMAIN_TIMER)

# PCM Low Latency, id 0
dnl PCM_PLAYBACK_ADD(name, pcm_id, playback)
PCM_PLAYBACK_ADD(Port0, 0, PIPELINE_PCM_1)

DAI_CONFIG(SAI, 1, 0, sai1-wm8960-hifi,
	SAI_CONFIG(I2S, SAI_CLOCK(mclk, 12288000, codec_mclk_in),
		SAI_CLOCK(bclk, 3072000, codec_master),
		SAI_CLOCK(fsync, 48000, codec_master),
		SAI_TDM(2, 32, 3, 3),
		SAI_CONFIG_DATA(SAI, 1, 0)))

DEBUG_END
