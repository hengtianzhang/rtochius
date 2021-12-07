/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SCHED_TASK_H_
#define __RTOCHIUS_SCHED_TASK_H_

#include <rtochius/sched.h>

#include <uapi/rtochius/magic.h>

extern struct task_struct init_task;

static inline void *task_stack_page(const struct task_struct *tsk)
{
	return tsk->stack;
}

static inline unsigned long *end_of_stack(const struct task_struct *tsk)
{
	return tsk->stack;
}

extern void set_task_stack_end_magic(struct task_struct *tsk);

#endif /* !__RTOCHIUS_SCHED_TASK_H_ */
