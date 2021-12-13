/*
 * Based on arch/arm/kernel/process.c
 *
 * Original Copyright (C) 1995  Linus Torvalds
 * Copyright (C) 1996-2000 Russell King - Converted to ARM.
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <rtochius/reboot.h>
#include <rtochius/sched.h>
#include <rtochius/thread_info.h>
#include <rtochius/mm_types.h>
#include <rtochius/percpu.h>

void (*pm_power_off)(void);

void (*arm_pm_restart)(enum reboot_mode reboot_mode, const char *cmd);

asmlinkage void ret_from_fork(void) asm("ret_from_fork");

void arch_release_task_struct(struct task_struct *tsk)
{
}

/*
 * Called from setup_new_exec() after (COMPAT_)SET_PERSONALITY.
 */
void arch_setup_new_exec(void)
{
	current->mm->context.flags = 0;
}

void tls_preserve_current_state(void)
{

}

unsigned long get_wchan(struct task_struct *p)
{
	return 0;
}


void release_thread(struct task_struct *dead_task)
{
}

/*
 * We store our current task in sp_el0, which is clobbered by userspace. Keep a
 * shadow copy so that we can restore this upon entry from userspace.
 *
 * This is *only* for exception entry from EL0, and is not valid until we
 * __switch_to() a user task.
 */
DEFINE_PER_CPU(struct task_struct *, __entry_task);
