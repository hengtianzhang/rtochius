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

#include <rtochius/boot_stat.h>
#include <rtochius/uts.h>
#include <rtochius/sched/task.h>
#include <rtochius/smp.h>
#include <rtochius/cpu.h>
#include <rtochius/irqflags.h>

#define COMMAND_LINE_SIZE	2048

/* Untouched command line saved by arch-specific code. */
static char __initdata boot_command_line[COMMAND_LINE_SIZE];

/* Untouched saved command line (eg. for /proc) */
char *saved_command_line;

asmlinkage __visible void __init start_kernel(void)
{
	set_task_stack_end_magic(&init_task);

	smp_setup_processor_id();

	local_irq_disable();

	boot_cpu_init();
	pr_notice("%s", rtochius_banner);
	setup_arch(boot_command_line);
}
