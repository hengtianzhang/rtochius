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
#include <rtochius/param.h>

/* Untouched command line saved by arch-specific code. */
char __initdata boot_command_line[COMMAND_LINE_SIZE];

/* Untouched saved command line (eg. for /proc) */
char *saved_command_line;

static int __init do_early_param(char *param, char *val,
					const char *unused, void *arg)
{
	const struct obs_kernel_param *p;

	for (p = __setup_start; p < __setup_end; p++) {
		if (parameq(param, p->str))
			if (p->setup_func(val) != 0)
				pr_warn("Malformed early option '%s'\n", param);
	}
	/* We accept everything at this stage. */
	return 0;
}

void __init parse_early_options(char *cmdline)
{
	parse_args("early options", cmdline, NULL, do_early_param);
}

/* Arch code calls this early on, or if not, just before other parsing. */
void __init parse_early_param(void)
{
	static int done __initdata;
	static char tmp_cmdline[COMMAND_LINE_SIZE] __initdata;

	if (done)
		return;

	/* All fall through to do_early_param. */
	strlcpy(tmp_cmdline, boot_command_line, COMMAND_LINE_SIZE);
	parse_early_options(tmp_cmdline);
	done = 1;
}

asmlinkage __visible void __init start_kernel(void)
{
	set_task_stack_end_magic(&init_task);

	smp_setup_processor_id();

	local_irq_disable();

	boot_cpu_init();
	pr_notice("%s", rtochius_banner);
	setup_arch(boot_command_line);

	pr_notice("Kernel command line: %s\n", boot_command_line);
	parse_early_options(boot_command_line);
}
