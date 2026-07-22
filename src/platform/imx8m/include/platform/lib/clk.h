/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020 NXP
 *
 * Author: Daniel Baluta <daniel.baluta@nxp.com>
 */

#ifdef __SOF_LIB_CLK_H__

#ifndef __PLATFORM_LIB_CLK_H__
#define __PLATFORM_LIB_CLK_H__

#include <stdint.h>

struct sof;

#define CLK_CPU(x)	(x)

#define CPU_DEFAULT_IDX		0

/*
 * i.MX8M runs the HiFi4 at a single fixed frequency, so the "lowest" CPU
 * frequency index is the only (default) one. Used by the IPC4 base firmware
 * when reporting the slow clock frequency.
 */
#define CPU_LOWEST_FREQ_IDX	CPU_DEFAULT_IDX

#define CLK_DEFAULT_CPU_HZ	800000000
#define CLK_MAX_CPU_HZ		800000000

#define NUM_CLOCKS	1

#define NUM_CPU_FREQ	1

/*
 * CAVS hardware config version reported over IPC4. i.MX is not a CAVS
 * platform, so report 0 (unspecified).
 */
#define HW_CFG_VERSION	0

void platform_clock_init(struct sof *sof);

#endif /* __PLATFORM_LIB_CLK_H__ */

#else

#error "This file shouldn't be included from outside of sof/lib/clk.h"

#endif /* __SOF_LIB_CLK_H__ */
