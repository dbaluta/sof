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

#include <sof/debug.h>
#include <sof/timer.h>
#include <sof/interrupt.h>
#include <sof/ipc.h>
#include <sof/mailbox.h>
#include <sof/sof.h>
#include <sof/stream.h>
#include <sof/dai.h>
#include <sof/dma.h>
#include <sof/alloc.h>
#include <sof/wait.h>
#include <sof/trace.h>
#include <sof/ssp.h>
#include <sof/drivers/printf.h>
#include <sof/drivers/peripheral.h>

#include <platform/interrupt.h>
#include <platform/mailbox.h>
#include <platform/shim.h>
#include <platform/dma.h>
#include <platform/platform.h>
#include <platform/mu.h>
#include <sof/audio/component.h>
#include <sof/audio/pipeline.h>
#include <uapi/ipc/header.h>

extern struct ipc *_ipc;

static void do_notify(void)
{
	uint32_t flags;
	struct ipc_msg *msg;
	struct mu_regs *base = (struct mu_regs *)MU_PADDR;

	spin_lock_irq(&_ipc->lock, flags);
	msg = _ipc->shared_ctx->dsp_msg;
	if (msg == NULL)
		goto out;

	tracev_ipc("ipc: not rx -> 0x%x", msg->header);

	/* copy the data returned from DSP */
	if (msg->rx_size && msg->rx_size < SOF_IPC_MSG_MAX_SIZE)
		mailbox_dspbox_read(msg->rx_data, 0, msg->rx_size);

	/* any callback ? */
	if (msg->cb)
		msg->cb(msg->cb_data, msg->rx_data);

	list_item_append(&msg->list, &_ipc->shared_ctx->empty_list);

out:
	spin_unlock_irq(&_ipc->lock, flags);

	mu_enableGIE(base, 1);
	/* clear DONE bit - tell Host we have completed */
	//shim_write(SHIM_IPCDH, shim_read(SHIM_IPCDH) & ~SHIM_IPCDH_DONE);

	/* unmask Done interrupt */
	//shim_write(SHIM_IMRD, shim_read(SHIM_IMRD) & ~SHIM_IMRD_DONE);
}

static void irq_handler(void *arg)
{
	uint32_t ctrl;
	uint32_t status;
	uint32_t msg = 0;
	struct mu_regs *base = (struct mu_regs *)MU_PADDR;

	/* Interrupt arrived, check src */
	ctrl = base->MU_CR;
	status = base->MU_SR;

	tracev_ipc("ipc: irq isr 0x%x", status);

	/* reply message(done) from host */
	if (status & IMX_MU_xSR_GIPn(1)) {
		/* Mask TE interrupt before return */
		mu_disableGIE(base, 1);
		mu_clearGIP(base, 1);
		interrupt_clear(PLATFORM_IPC_INTERRUPT);
		do_notify();
	}

	/* new message from host */
	if (status & IMX_MU_xSR_GIPn(0)) {
		interrupt_clear(PLATFORM_IPC_INTERRUPT);

		mu_disableGIE(base, 0);
		mu_clearGIP(base, 0);

		/* TODO: place message in Q and process later */
		/* It's not Q ATM, may overwrite */
		if (_ipc->host_pending) {
			trace_ipc_error("ipc: dropping msg 0x%x", msg);
			trace_ipc_error(" isr 0x%x ctrl 0x%x ipcxh 0x%x",
					status, ctrl);
		} else {
			_ipc->host_pending = 1;
			ipc_schedule_process(_ipc);
		}
	}
}

void ipc_platform_do_cmd(struct ipc *ipc)
{
	struct ipc_data *iipc = ipc_get_drvdata(ipc);
	struct sof_ipc_reply reply;
	//uint32_t ipcxh;
	int32_t err;
	struct mu_regs *base = (struct mu_regs *)MU_PADDR;

	/* perform command and return any error */
	err = ipc_cmd();
	if (err > 0) {
		goto done; /* reply created and copied by cmd() */
	} else {
		/* send std error reply */
		reply.error = err;
	}

	/* send std error/ok reply */
	reply.hdr.cmd = SOF_IPC_GLB_REPLY;
	reply.hdr.size = sizeof(reply);
	mailbox_hostbox_write(0, &reply, sizeof(reply));

done:
	ipc->host_pending = 0;

	/* clear BUSY bit and set DONE bit - accept new messages */
//	ipcxh = shim_read(SHIM_IPCXH);
//	ipcxh &= ~SHIM_IPCXH_BUSY;
//	ipcxh |= SHIM_IPCXH_DONE;
//	shim_write(SHIM_IPCXH, ipcxh);

	/* unmask busy interrupt */
//	shim_write(SHIM_IMRD, shim_read(SHIM_IMRD) & ~SHIM_IMRD_BUSY);


	mu_enableGIE(base, 0);
	mu_enableinterrupt_gir(base, 0);
	// TODO: signal audio work to enter D3 in normal context
	/* are we about to enter D3 ? */
	if (iipc->pm_prepare_D3) {
		while (1) {
			wait_for_interrupt(0);
		}
	}
}

void ipc_platform_send_msg(struct ipc *ipc)
{
	struct ipc_msg *msg;
	uint32_t flags;

	spin_lock_irq(&ipc->lock, flags);

	/* any messages to send ? */
	if (list_is_empty(&ipc->shared_ctx->msg_list)) {
		ipc->shared_ctx->dsp_pending = 0;
		goto out;
	}

	/* Check if we can set message, check for TE bit? */
#if 0
	/* can't send notification when one is in progress */
	if (shim_read(SHIM_IPCDH) & (SHIM_IPCDH_BUSY | SHIM_IPCDH_DONE))
		goto out;
#endif
	/* now send the message */
	msg = list_first_item(&ipc->shared_ctx->msg_list, struct ipc_msg,
			      list);
	mailbox_dspbox_write(0, msg->tx_data, msg->tx_size);
	list_item_del(&msg->list);
	ipc->shared_ctx->dsp_msg = msg;
	tracev_ipc("ipc: msg tx -> 0x%x", msg->header);

	/* now interrupt host to tell it we have message sent */

	imx_mu_write(IMX_MU_xTRn(0), msg->header);
//	shim_write(SHIM_IPCDL, msg->header);
//	shim_write(SHIM_IPCDH, SHIM_IPCDH_BUSY);

	list_item_append(&msg->list, &ipc->shared_ctx->empty_list);

out:
	spin_unlock_irq(&ipc->lock, flags);
}

int platform_ipc_init(struct ipc *ipc)
{
	struct ipc_data *iipc;
	int ret;
	struct mu_regs *base = (struct mu_regs *)MU_PADDR;

	//uint32_t imrd, dir, caps, dev;
	//struct mu_regs *base = (struct mu_regs *)MU_PADDR;

	_ipc = ipc;

	/* init ipc data */
	iipc = rzalloc(RZONE_SYS, SOF_MEM_CAPS_RAM,
		sizeof(struct ipc_data));
	ipc_set_drvdata(_ipc, iipc);

	/* schedule */
	schedule_task_init(&_ipc->ipc_task, ipc_process_task, _ipc);
	schedule_task_config(&_ipc->ipc_task, TASK_PRI_IPC, 0);

#ifdef CONFIG_HOST_PTABLE
	/* allocate page table buffer */
	iipc->page_table = rzalloc(RZONE_SYS, SOF_MEM_CAPS_RAM,
		PLATFORM_PAGE_TABLE_SIZE);
	if (iipc->page_table)
		bzero(iipc->page_table, PLATFORM_PAGE_TABLE_SIZE);
#endif
#if 0
	/* request HDA DMA with shared access privilege */
	caps = 0;
	dir = DMA_DIR_HMEM_TO_LMEM;
	dev = DMA_DEV_HOST;
	iipc->dmac = dma_get(dir, caps, dev, DMA_ACCESS_SHARED);

	/* PM */
	iipc->pm_prepare_D3 = 0;
#endif
	/* configure interrupt */
	ret = PLATFORM_IPC_INTERRUPT;
	interrupt_register(PLATFORM_IPC_INTERRUPT, IRQ_AUTO_UNMASK,
			   irq_handler, NULL);
	interrupt_enable(PLATFORM_IPC_INTERRUPT);

	mu_enableGIE(base, 0);
	mu_enableGIE(base, 1);

	//mu_enableinterrupt_gir(base, 0);

	/* Unmask TIE and RIE interrupts */
	//imx_mu_xcr_rmw(IMX_MU_xCR_TIEn(0) | IMX_MU_xCR_RIEn(0), 0);
	return 0;
}

