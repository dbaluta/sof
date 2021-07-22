/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019 NXP
 *
 * Author: Daniel Baluta <daniel.baluta@nxp.com>
 */

#ifdef __SOF_DRIVERS_INTERRUPT_H__

#ifndef __PLATFORM_DRIVERS_INTERRUPT_H__
#define __PLATFORM_DRIVERS_INTERRUPT_H__

#include <sof/bit.h>


/* IRQ numbers */
#define IRQ_DSP_SYS_NMI      0
#define IRQ_NUM_SOFTWARE0    1
#define IRQ_NUM_TIMER0       2
#define IRQ_NUM_TIMER1       3
#define IRQ_NUM_MU2          14
#define IRQ_NUM_MU3          15
#define IRQ_SAI5_NUM         23
#define IRQ_SAI6_NUM         24
#define IRQ_SAI7_NUM         25

#define PLATFORM_IRQ_HW_NUM	XCHAL_NUM_INTERRUPTS
#define PLATFORM_IRQ_CHILDREN	0 /* Each cascaded struct covers 64 IRQs */
/* IMX: Covered steer IRQs are modulo-64 aligned. */
#define PLATFORM_IRQ_FIRST_CHILD  0

/* no irqstr in 8ulp */
int irqstr_get_sof_int(int irqstr_int);
#endif /* __PLATFORM_DRIVERS_INTERRUPT_H__ */

#else

#error "This file shouldn't be included from outside of sof/drivers/interrupt.h"

#endif /* __SOF_DRIVERS_INTERRUPT_H__ */
