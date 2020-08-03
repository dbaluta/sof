# Low Latency Passthrough with volume Pipeline and PCM
#
# Pipeline Endpoints for connection are :-
#
#  host PCM_P --> B0 --> MP3 0 --> B1 --> sink DAI0

# Include topology builder
include(`utils.m4')
include(`buffer.m4')
include(`pcm.m4')
include(`dai.m4')
include(`bytecontrol.m4')
include(`pipeline.m4')
include(`post_process.m4')

#
# Components and Buffers
#

# Host "Post Process Playback" PCM
# with 2 sink and 0 source periods
W_PCM_PLAYBACK(PCM_ID, Post Process Playback, 2, 0, SCHEDULE_CORE)

# "Post Process 0" has x sink period and 2 source periods
W_POST_PROCESS(0, PIPELINE_FORMAT, DAI_PERIODS, 2, SCHEDULE_CORE)

# Playback Buffers
W_BUFFER(0, COMP_BUFFER_SIZE(2,
	COMP_SAMPLE_SIZE(PIPELINE_FORMAT), PIPELINE_CHANNELS,
	COMP_PERIOD_FRAMES(PCM_MAX_RATE, SCHEDULE_PERIOD)),
	PLATFORM_HOST_MEM_CAP)
W_BUFFER(1, COMP_BUFFER_SIZE(DAI_PERIODS,
	COMP_SAMPLE_SIZE(DAI_FORMAT), PIPELINE_CHANNELS,
	COMP_PERIOD_FRAMES(PCM_MAX_RATE, SCHEDULE_PERIOD)),
	PLATFORM_DAI_MEM_CAP)

#
# Pipeline Graph
#
# Host Playback --> B0 --> POST_PROCESS 0 --> B1 --> sink DAI0

P_GRAPH(pipe-pass-postprocess-playback-PIPELINE_ID, PIPELINE_ID,
	LIST(`		',
	`dapm(N_BUFFER(0), N_PCMP(PCM_ID))',
	`dapm(N_POST_PROCESS(0), N_BUFFER(0))',
	`dapm(N_BUFFER(1), N_POST_PROCESS(0))'))

#
# Pipeline Source and Sinks
#
indir(`define', concat(`PIPELINE_SOURCE_', PIPELINE_ID), N_BUFFER(1))
indir(`define', concat(`PIPELINE_PCM_', PIPELINE_ID),
	Post Process Playback PCM_ID)

#
# PCM Configuration

#
PCM_CAPABILITIES(Post Process Playback PCM_ID, `S32_LE,S24_LE,S16_LE',
	PCM_MIN_RATE, PCM_MAX_RATE, 2, PIPELINE_CHANNELS,
	2, 16, 192, 16384, 65536, 65536)

