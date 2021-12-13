/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_IRQ_H_
#define __RTOCHIUS_IRQ_H_

#include <asm/cache.h>
#include <asm/ptrace.h>

/*
 * Allows interrupt handlers to find the irqchip that's been registered as the
 * top-level IRQ handler.
 */
extern void (*handle_arch_irq)(struct pt_regs *) __ro_after_init;

#endif /* !__RTOCHIUS_IRQ_H_ */
