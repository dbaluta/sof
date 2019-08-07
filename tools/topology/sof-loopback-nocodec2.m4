# Loopback pipeline with 2 PCMs tied together with a single buffer
#
# Pipeline endpoints are:
#
# host PCM_P -> B0 -> host PCM_C

# Include topology builder
include(`utils.m4')
include(`pcm.m4')
include(`pga.m4')
include(`dai.m4')
include(`mixercontrol.m4')
include(`pipeline.m4')
include(`sai.m4')
include(`pcm.m4')

#include TLV library
include(`common/tlv.m4')
include(`sof/tokens.m4')
include(`buffer.m4')
include(`mixer.m4')

include(`platform/imx/imx8.m4')
#
# Components and buffers
#
undefine(`PIPELINE_ID')define(`PIPELINE_ID', 1)
# Host "Passthrough Playback" PCM
# with 2 sink and 0 source periods
W_PCM_PLAYBACK(0, Passthrough Playback, 2, 0)
undefine(`PIPELINE_ID')define(`PIPELINE_ID', 2)
# Host "Passthrough Capture" PCM
# with 2 source and 0 sink periods
W_PCM_CAPTURE(0, Passthrough Capture, 0, 2)
undefine(`PIPELINE_ID')define(`PIPELINE_ID', 1)
include(`buffer.m4')

# Shared buffer
W_BUFFER(0, COMP_BUFFER_SIZE(2,
	COMP_SAMPLE_SIZE(s32le), 2, 48),
	PLATFORM_HOST_MEM_CAP)

# Pipeline Graph P1
#
# host PCM_P --> B0

P_GRAPH(pipe-loopback-1, 1,
	LIST(`		',
	`dapm(N_BUFFER(0), N_PCMP(0))'))

define(`PIPELINE_SOURCE_1', N_BUFFER(0))
define(`PIPELINE_PCM_1', N_PCMP(0))

PCM_CAPABILITIES(Passthrough Playback 0, `S32_LE,S24_LE,S16_LE', 48000, 48000, 2, 2, 2, 16, 192, 16384, 65536, 65536)

W_PIPELINE(N_PCMP(0), 1000, 0, 48, 0, 0, pipe_media_schedule_plat)

define(`PIPELINE_SINK_2', N_BUFFER(0))
define(`PIPELINE_PCM_2', N_PCMC(0))

# Pipeline Graph P2
#
# host PCM_C <-- B0
P_GRAPH(pipe-loopback-2, 2,
	LIST(`		',
	`dapm(N_PCMC(0), N_BUFFER(0))'))

undefine(`PIPELINE_ID')define(`PIPELINE_ID', 2)

PCM_CAPABILITIES(Passthrough Capture 0, `S32_LE,S24_LE,S16_LE', 48000, 48000, 2, 2, 2, 16, 192, 16384, 65536, 65536)

# Pipeline component

W_PIPELINE(N_PCMC(0), 1000, 0, 48, 0, 0, pipe_media_schedule_plat)

PCM_PLAYBACK_ADD(Port5, 0, Passthrough Playback 0)
PCM_CAPTURE_ADD(Port5, 0, Passthrough Capture 0)
