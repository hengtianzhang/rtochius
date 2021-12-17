/* SPDX-License-Identifier: GPL-2.0 */
/* interrupt.h */
#ifndef __RTOCHIUS_INTERRUPT_H_
#define __RTOCHIUS_INTERRUPT_H_

#include <rtochius/cpumask.h>

/*
 * We want to know which function is an entrypoint of a hardirq or a softirq.
 */
#define __irq_entry		 __attribute__((__section__(".irqentry.text")))
#define __softirq_entry  \
	__attribute__((__section__(".softirqentry.text")))

/*
 * It is safe to do non-atomic ops on ->hardirq_context,
 * because NMI handlers may not preempt and the ops are
 * always balanced, so the interrupted value of ->hardirq_context
 * will always be restored.
 */
#define __irq_enter()					\
	do {						\
		preempt_count_add(HARDIRQ_OFFSET);	\
	} while (0)

/*
 * Enter irq context (on NO_HZ, update jiffies):
 */
extern void irq_enter(void);
extern void irq_exit(void);

/*
 * These correspond to the IORESOURCE_IRQ_* defines in
 * linux/ioport.h to select the interrupt line behaviour.  When
 * requesting an interrupt without specifying a IRQF_TRIGGER, the
 * setting should be assumed to be "as already configured", which
 * may be as per machine or firmware initialisation.
 */
#define IRQF_TRIGGER_NONE	0x00000000
#define IRQF_TRIGGER_RISING	0x00000001
#define IRQF_TRIGGER_FALLING	0x00000002
#define IRQF_TRIGGER_HIGH	0x00000004
#define IRQF_TRIGGER_LOW	0x00000008
#define IRQF_TRIGGER_MASK	(IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW | \
				 IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)
#define IRQF_TRIGGER_PROBE	0x00000010

/*
 * These flags used only by the kernel as part of the
 * irq handling routines.
 *
 * IRQF_SHARED - allow sharing the irq among several devices
 * IRQF_PROBE_SHARED - set by callers when they expect sharing mismatches to occur
 * IRQF_TIMER - Flag to mark this interrupt as timer interrupt
 * IRQF_PERCPU - Interrupt is per cpu
 * IRQF_NOBALANCING - Flag to exclude this interrupt from irq balancing
 * IRQF_IRQPOLL - Interrupt is used for polling (only the interrupt that is
 *                registered first in a shared interrupt is considered for
 *                performance reasons)
 * IRQF_ONESHOT - Interrupt is not reenabled after the hardirq handler finished.
 *                Used by threaded interrupts which need to keep the
 *                irq line disabled until the threaded handler has been run.
 * IRQF_NO_SUSPEND - Do not disable this IRQ during suspend.  Does not guarantee
 *                   that this interrupt will wake the system from a suspended
 *                   state.  See Documentation/power/suspend-and-interrupts.txt
 * IRQF_FORCE_RESUME - Force enable it on resume even if IRQF_NO_SUSPEND is set
 * IRQF_NO_THREAD - Interrupt cannot be threaded
 * IRQF_EARLY_RESUME - Resume IRQ early during syscore instead of at device
 *                resume time.
 * IRQF_COND_SUSPEND - If the IRQ is shared with a NO_SUSPEND user, execute this
 *                interrupt handler after suspending interrupts. For system
 *                wakeup devices users need to implement wakeup detection in
 *                their interrupt handlers.
 */
#define IRQF_SHARED		0x00000080
#define IRQF_PROBE_SHARED	0x00000100
#define __IRQF_TIMER		0x00000200
#define IRQF_PERCPU		0x00000400
#define IRQF_NOBALANCING	0x00000800
#define IRQF_IRQPOLL		0x00001000
#define IRQF_ONESHOT		0x00002000
#define IRQF_NO_SUSPEND		0x00004000
#define IRQF_FORCE_RESUME	0x00008000
#define IRQF_NO_THREAD		0x00010000
#define IRQF_EARLY_RESUME	0x00020000
#define IRQF_COND_SUSPEND	0x00040000

#define IRQF_TIMER		(__IRQF_TIMER | IRQF_NO_SUSPEND | IRQF_NO_THREAD)

/*
 * These values can be returned by request_any_context_irq() and
 * describe the context the interrupt will be run in.
 *
 * IRQC_IS_HARDIRQ - interrupt runs in hardirq context
 * IRQC_IS_NESTED - interrupt runs in a nested threaded context
 */
enum {
	IRQC_IS_HARDIRQ	= 0,
	IRQC_IS_NESTED,
};

typedef irqreturn_t (*irq_handler_t)(int, void *);

/**
 * struct irqaction - per interrupt action descriptor
 * @handler:	interrupt handler function
 * @name:	name of the device
 * @dev_id:	cookie to identify the device
 * @percpu_dev_id:	cookie to identify the device
 * @next:	pointer to the next irqaction for shared interrupts
 * @irq:	interrupt number
 * @flags:	flags (see IRQF_* above)
 * @thread_fn:	interrupt handler function for threaded interrupts
 * @thread:	thread pointer for threaded interrupts
 * @secondary:	pointer to secondary irqaction (force threading)
 * @thread_flags:	flags related to @thread
 * @thread_mask:	bitmask for keeping track of @thread activity
 * @dir:	pointer to the proc/irq/NN/name entry
 */
struct irqaction {
	irq_handler_t		handler;
	void			*dev_id;
	void __percpu		*percpu_dev_id;
	struct irqaction	*next;
	irq_handler_t		thread_fn;
	struct task_struct	*thread;
	struct irqaction	*secondary;
	unsigned int		irq;
	unsigned int		flags;
	unsigned long		thread_flags;
	unsigned long		thread_mask;
	const char		*name;
} ____cacheline_internodealigned_in_smp;

/*
 * If a (PCI) device interrupt is not connected we set dev->irq to
 * IRQ_NOTCONNECTED. This causes request_irq() to fail with -ENOTCONN, so we
 * can distingiush that case from other error returns.
 *
 * 0x80000000 is guaranteed to be outside the available range of interrupts
 * and easy to distinguish from other possible incorrect values.
 */
#define IRQ_NOTCONNECTED	(1U << 31)








extern cpumask_var_t irq_default_affinity;


/**
 * struct irq_affinity_desc - Interrupt affinity descriptor
 * @mask:	cpumask to hold the affinity assignment
 * @is_managed: 1 if the interrupt is managed internally
 */
struct irq_affinity_desc {
	struct cpumask	mask;
	unsigned int	is_managed : 1;
};

#endif /* !__RTOCHIUS_INTERRUPT_H_ */
