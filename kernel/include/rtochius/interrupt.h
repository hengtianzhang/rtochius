/* SPDX-License-Identifier: GPL-2.0 */
/* interrupt.h */
#ifndef __RTOCHIUS_INTERRUPT_H_
#define __RTOCHIUS_INTERRUPT_H_

/*
 * We want to know which function is an entrypoint of a hardirq or a softirq.
 */
#define __irq_entry		 __attribute__((__section__(".irqentry.text")))
#define __softirq_entry  \
	__attribute__((__section__(".softirqentry.text")))

#endif /* !__RTOCHIUS_INTERRUPT_H_ */
