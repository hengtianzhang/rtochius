// SPDX-License-Identifier: GPL-2.0
#include <rtochius/sched.h>

#define __init_task_data __attribute__((__section__(".data..init_task")))

/*
 * Set up the first task table, touch at your own risk!. Base=0,
 * limit=0x1fffff (=2MB)
 */
struct task_struct init_task
	__init_task_data
= {
	.thread_info	= INIT_THREAD_INFO(init_task),
};
