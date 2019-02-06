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
 */

#include <platform/memory.h>
#include <platform/mailbox.h>
#include <platform/shim.h>
#include <platform/dma.h>
#include <platform/dai.h>
#include <platform/clk.h>
#include <platform/timer.h>
#include <platform/pmc.h>
#include <platform/mu.h>
#include <platform/platcfg.h>
#include <uapi/ipc/info.h>
#include <sof/mailbox.h>
#include <sof/dai.h>
#include <sof/drivers/printf.h>
#include <sof/drivers/peripheral.h>
#include <sof/dma.h>
#include <sof/interrupt.h>
#include <sof/sof.h>
#include <sof/work.h>
#include <sof/clk.h>
#include <sof/ipc.h>
#include <sof/trace.h>
#include <sof/agent.h>
#include <sof/dma-trace.h>
#include <sof/audio/component.h>
#include <sof/cpu.h>
#include <sof/notifier.h>
#include <config.h>
#include <string.h>
#include <version.h>

static const struct sof_ipc_fw_ready ready
	__attribute__((section(".fw_ready"))) = {
	.hdr = {
		.cmd = SOF_IPC_FW_READY,
		.size = sizeof(struct sof_ipc_fw_ready),
	},
	/* dspbox is for DSP initiated IPC, hostbox is for host initiated IPC */
	.version = {
		.hdr.size = sizeof(struct sof_ipc_fw_version),
		.micro = SOF_MICRO,
		.minor = SOF_MINOR,
		.major = SOF_MAJOR,
#ifdef DEBUG_BUILD
		/* only added in debug for reproducability in releases */
		.build = SOF_BUILD,
		.date = __DATE__,
		.time = __TIME__,
#endif
		.tag = SOF_TAG,
		.abi_version = SOF_ABI_VERSION,
	},
	.debug = DEBUG_SET_FW_READY_FLAGS
};

#define NUM_IMX_WINDOWS		6

static const struct sof_ipc_window sram_window = {
	.ext_hdr	= {
		.hdr.cmd = SOF_IPC_FW_READY,
		.hdr.size = sizeof(struct sof_ipc_window) +
			sizeof(struct sof_ipc_window_elem) * NUM_IMX_WINDOWS,
		.type	= SOF_IPC_EXT_WINDOW,
	},
	.num_windows	= NUM_IMX_WINDOWS,
	.window	= {
		{
			.type	= SOF_IPC_REGION_UPBOX,
			.id	= 0,	/* map to host window 0 */
			.flags	= 0, // TODO: set later
			.size	= MAILBOX_DSPBOX_SIZE,
			.offset	= MAILBOX_DSPBOX_OFFSET,
		},
		{
			.type	= SOF_IPC_REGION_DOWNBOX,
			.id	= 0,	/* map to host window 0 */
			.flags	= 0, // TODO: set later
			.size	= MAILBOX_HOSTBOX_SIZE,
			.offset	= MAILBOX_HOSTBOX_OFFSET,
		},
		{
			.type	= SOF_IPC_REGION_DEBUG,
			.id	= 0,	/* map to host window 0 */
			.flags	= 0, // TODO: set later
			.size	= MAILBOX_DEBUG_SIZE,
			.offset	= MAILBOX_DEBUG_OFFSET,
		},
		{
			.type	= SOF_IPC_REGION_TRACE,
			.id	= 0,	/* map to host window 0 */
			.flags	= 0, // TODO: set later
			.size	= MAILBOX_TRACE_SIZE,
			.offset	= MAILBOX_TRACE_OFFSET,
		},
		{
			.type	= SOF_IPC_REGION_STREAM,
			.id	= 0,	/* map to host window 0 */
			.flags	= 0, // TODO: set later
			.size	= MAILBOX_STREAM_SIZE,
			.offset	= MAILBOX_STREAM_OFFSET,
		},
		{
			.type	= SOF_IPC_REGION_EXCEPTION,
			.id	= 0,	/* map to host window 0 */
			.flags	= 0, // TODO: set later
			.size	= MAILBOX_EXCEPTION_SIZE,
			.offset	= MAILBOX_EXCEPTION_OFFSET,
		},
	},
};

struct timer *platform_timer;


int platform_boot_complete(uint32_t boot_message)
{
	__dsp_printf("imx8, platform_boot_complete, begin\n");
	mailbox_dspbox_write(0, &ready, sizeof(ready));
	mailbox_dspbox_write(sizeof(ready), &sram_window,
			     sram_window.ext_hdr.hdr.size);

	struct mu_regs *base = (struct mu_regs *)MU_PADDR;
	__dsp_printf("ready hdr %x size %d\n", ready.hdr.cmd, ready.hdr.size);
	mu_enableinterrupt_gir(base, 1);
#if 0
	__dsp_printf("imx8, wrote to dspbox\n");
	/* now interrupt host to tell it we are done booting */
//	shim_write(SHIM_IPCDL, SOF_IPC_FW_READY | outbox);
//	shim_write(SHIM_IPCDH, SHIM_IPCDH_BUSY);
//
	__dsp_printf("STATUS BEFORE %x\n", imx_mu_read(IMX_MU_xSR));
	imx_mu_xcr_rmw(IMX_MU_xCR_GIEn(0), 0);
	imx_mu_xcr_rmw(IMX_MU_xCR_GIEn(1), 0);
	imx_mu_xcr_rmw(IMX_MU_xCR_GIEn(2), 0);
	imx_mu_xcr_rmw(IMX_MU_xCR_GIEn(3), 0);

	__dsp_printf("STATUS AFTER %x\n", imx_mu_read(IMX_MU_xSR));

	//imx_mu_write(IMX_MU_xTRn(0), SOF_IPC_FW_READY);
	//
	/* boot now complete so we can relax the CPU */
	/* For now skip this to gain more processing performance
	 * for SRC component.
	 */
	/* clock_set_freq(CLK_CPU, CLK_DEFAULT_CPU_HZ); */
#endif
	return 0;
}

int platform_init(struct sof *sof)
{
	ipc_init(sof);

	return 0;
}
