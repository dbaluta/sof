/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2018 Intel Corporation. All rights reserved.
 *
 * Author: Janusz Jankowski <janusz.jankowski@linux.intel.com>
 */

#ifndef __SOF_DRIVERS_TIMER_H__
#define __SOF_DRIVERS_TIMER_H__

#include <stdint.h>
#include <sof/lib/cpu.h>
#include <sof/sof.h>
#include <sof/platform.h>

struct comp_dev;
struct sof_ipc_stream_posn;

#ifndef __ZEPHYR__

#include <arch/drivers/timer.h>

#define TIMER0	0
#define TIMER1	1
#define TIMER2	2
#define TIMER3	3
#define TIMER4	4

int timer_register(struct timer *timer, void (*handler)(void *arg), void *arg);
void timer_unregister(struct timer *timer, void *arg);
void timer_enable(struct timer *timer, void *arg, int core);
void timer_disable(struct timer *timer, void *arg, int core);

static inline struct timer *timer_get(void)
{
	return sof_get()->platform_timer;
}

static inline struct timer *cpu_timer_get(void)
{
	return &(sof_get()->cpu_timers[cpu_get_id()]);
}

static inline int64_t timer_set(struct timer *timer, uint64_t ticks)
{
	return arch_timer_set(timer, ticks);
}

void timer_set_ms(struct timer *timer, unsigned int ms);

static inline void timer_clear(struct timer *timer)
{
	arch_timer_clear(timer);
}

unsigned int timer_get_count(struct timer *timer);

unsigned int timer_get_count_delta(struct timer *timer);

static inline uint64_t timer_get_system(struct timer *timer)
{
	return arch_timer_get_system(timer);
}

int64_t platform_timer_set(struct timer *timer, uint64_t ticks);
void platform_timer_clear(struct timer *timer);
uint64_t platform_timer_get(struct timer *timer);
uint64_t platform_timer_get_atomic(struct timer *timer);

static inline uint64_t platform_safe_get_time(struct timer *timer)
{
	/* Default to something small but at least 1.0 microsecond so it
	 * does not look like an uninitialized zero; not even when the
	 * user does not request any microseconds decimals. See
	 * DEFAULT_CLOCK constant in logger.c
	 */
	return timer ? platform_timer_get(timer) : 50;
}

void platform_timer_start(struct timer *timer);
void platform_timer_stop(struct timer *timer);

#define k_ms_to_cyc_ceil64(ms)		clock_ms_to_ticks(PLATFORM_DEFAULT_CLOCK, ms)
#define k_us_to_cyc_ceil64(us)		clock_us_to_ticks(PLATFORM_DEFAULT_CLOCK, us)
#define k_ns_to_cyc_near64(ns)		clock_ns_to_ticks(PLATFORM_DEFAULT_CLOCK, ns)

#define k_cyc_to_ms_near64(ticks)	((ticks) / clock_ms_to_ticks(PLATFORM_DEFAULT_CLOCK, 1))
#define k_cyc_to_us_near64(ticks)	((ticks) / clock_us_to_ticks(PLATFORM_DEFAULT_CLOCK, 1))

#define k_cycle_get_64()		platform_timer_get(timer_get())
#define k_cycle_get_64_atomic()		platform_timer_get_atomic(timer_get())
#define k_cycle_get_64_safe()		platform_safe_get_time(timer_get())
#else
#define k_cycle_get_64_safe()		k_cycle_get_64()
#define k_cycle_get_64_atomic()		k_cycle_get_64()
#define platform_timer_stop(x)
#endif /* __ZEPHYR__ */

/* get timestamp for host stream DMA position */
void platform_host_timestamp(struct comp_dev *host,
			     struct sof_ipc_stream_posn *posn);

/* get timestamp for DAI stream DMA position */
void platform_dai_timestamp(struct comp_dev *dai,
			    struct sof_ipc_stream_posn *posn);

/* get current wallclock for componnent */
void platform_dai_wallclock(struct comp_dev *dai, uint64_t *wallclock);

#endif /* __SOF_DRIVERS_TIMER_H__ */
