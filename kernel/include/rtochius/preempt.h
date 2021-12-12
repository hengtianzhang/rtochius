/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_PREEMPT_H_
#define __RTOCHIUS_PREEMPT_H_

#define preempt_disable()
#define preempt_enable()

#define in_interrupt()		(0)

#define preempt_count_add(x)

#define SOFTIRQ_LOCK_OFFSET (1)

#define preemptible()	(0)

#endif /* !__RTOCHIUS_PREEMPT_H_ */
