/*
 *  linux/init/main.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  GK 2/5/95  -  Changed to support mounting root fs via NFS
 *  Added initrd & change_root: Werner Almesberger & Hans Lermen, Feb '96
 *  Moan early if gcc is old, avoiding bogus kernels - Paul Gortmaker, May '96
 *  Simplified starting of init:  Michael A. Griffith <grif@acm.org>
 */
#include <base/compiler.h>
#include <base/linkage.h>
#include <base/init.h>
#include <base/types.h>

#include <rtochius/sched/task.h>
#include <rtochius/system_stat.h>
#include <rtochius/smp.h>
#include <rtochius/irqflags.h>

asmlinkage __visible void __init start_kernel(void)
{
	set_task_stack_end_magic(&init_task);

	smp_setup_processor_id();

	local_irq_disable();
}
