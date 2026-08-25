/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2024 NXP
 */

#ifdef __SOF_LIB_MAILBOX_H__

#ifndef __PLATFORM_LIB_MAILBOX_H__
#define __PLATFORM_LIB_MAILBOX_H__

#include <rtos/cache.h>
#include <stddef.h>
#include <stdint.h>

/* i.MX95's mailbox is organized as follows:
 *
 * +---------------+---------------+---------+
 * | Region name   | Base address  |  Size   |
 * +---------------+---------------+---------+
 * | Inbox region  |   0x86000000  | 0x1000  |
 * +---------------+---------------+---------+
 * | Outbox region |   0x86001000  | 0x1000  |
 * +---------------+---------------+---------+
 * | Stream region |   0x86002000  | 0x1000  |
 * +---------------+---------------+---------+
 * | Debug region  |   0x86003000  | 0x0800  |
 * +---------------+---------------+---------+
 * | SW reg region |   0x86003800  | 0x0800  |
 * +---------------+---------------+---------+
 *
 * IMPORTANT: all regions should be 32-byte aligned.
 * This is required because cache maintenance might
 * be performed on them.
 *
 * The IPC4 compact 8-byte header is carried in the bytes preceding each
 * payload box (i.MX has no MU data registers for SOF); the payload bases are
 * shifted by the header size while the header slots stay at the region base.
 * For IPC3 the header size is 0.
 */
#if CONFIG_IPC_MAJOR_4
#define SRAM_IPC4_HDR_SIZE	8
#else
#define SRAM_IPC4_HDR_SIZE	0
#endif

/* inbox (host -> DSP / hostbox) */
#define MAILBOX_HOSTBOX_HDR_BASE 0x86000000
#define MAILBOX_HOSTBOX_SIZE (0x1000 - SRAM_IPC4_HDR_SIZE)
#define MAILBOX_HOSTBOX_BASE (0x86000000 + SRAM_IPC4_HDR_SIZE)
#define MAILBOX_HOSTBOX_OFFSET SRAM_IPC4_HDR_SIZE

/* outbox (DSP -> host / dspbox) */
#define MAILBOX_DSPBOX_HDR_BASE 0x86001000
#define MAILBOX_DSPBOX_SIZE (0x1000 - SRAM_IPC4_HDR_SIZE)
#define MAILBOX_DSPBOX_BASE (0x86001000 + SRAM_IPC4_HDR_SIZE)
#define MAILBOX_DSPBOX_OFFSET (0x1000 + SRAM_IPC4_HDR_SIZE)

/* stream */
#define MAILBOX_STREAM_SIZE 0x1000
#define MAILBOX_STREAM_BASE 0x86002000
#define MAILBOX_STREAM_OFFSET 0x2000

/* debug */
#define MAILBOX_DEBUG_SIZE 0x800
#define MAILBOX_DEBUG_BASE 0x86003000
#define MAILBOX_DEBUG_OFFSET 0x3000

/* IPC4 SW registers (copier LLP); carved out of mailbox SRAM */
#define MAILBOX_SW_REG_SIZE 0x800
#define MAILBOX_SW_REG_BASE 0x86003800
#define MAILBOX_SW_REG_OFFSET 0x3800

static inline void mailbox_sw_reg_write(size_t offset, uint32_t src)
{
	volatile uint32_t *ptr;

	ptr = (volatile uint32_t *)(MAILBOX_SW_REG_BASE + offset);
	*ptr = src;
	dcache_writeback_region((__sparse_force void __sparse_cache *)(MAILBOX_SW_REG_BASE +
								       offset),
				sizeof(src));
}

static inline void mailbox_sw_reg_write64(size_t offset, uint64_t src)
{
	volatile uint64_t *ptr;

	ptr = (volatile uint64_t *)(MAILBOX_SW_REG_BASE + offset);
	*ptr = src;
	dcache_writeback_region((__sparse_force void __sparse_cache *)(MAILBOX_SW_REG_BASE +
								       offset),
				sizeof(src));
}

static inline uint32_t mailbox_sw_reg_read(size_t offset)
{
	volatile uint32_t *ptr;

	ptr = (volatile uint32_t *)(MAILBOX_SW_REG_BASE + offset);
	dcache_invalidate_region((__sparse_force void __sparse_cache *)(MAILBOX_SW_REG_BASE +
								       offset),
				 sizeof(uint32_t));

	return *ptr;
}

static inline uint64_t mailbox_sw_reg_read64(size_t offset)
{
	volatile uint64_t *ptr;

	ptr = (volatile uint64_t *)(MAILBOX_SW_REG_BASE + offset);
	dcache_invalidate_region((__sparse_force void __sparse_cache *)(MAILBOX_SW_REG_BASE +
								       offset),
				 sizeof(uint64_t));

	return *ptr;
}

static inline void mailbox_sw_regs_write(size_t offset, const void *src, size_t bytes)
{
	volatile uint8_t *dst = (volatile uint8_t *)(MAILBOX_SW_REG_BASE + offset);
	const uint8_t *s = src;
	size_t i;

	for (i = 0; i < bytes; i++)
		dst[i] = s[i];

	dcache_writeback_region((__sparse_force void __sparse_cache *)(MAILBOX_SW_REG_BASE +
								       offset),
				bytes);
}

#endif /* __PLATFORM_LIB_MAILBOX_H__ */

#else

#error "This file shouldn't be included from outside of sof/lib/mailbox.h"

#endif /* __SOF_LIB_MAILBOX_H__ */
