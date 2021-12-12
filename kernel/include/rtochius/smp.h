/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SMP_H_
#define __RTOCHIUS_SMP_H_

/*
 *	Generic SMP support
 *		Alan Cox. <alan@redhat.com>
 */
#include <asm/smp.h>

extern void smp_setup_processor_id(void);

extern void setup_nr_cpu_ids(void);

extern int __boot_cpu_id;

static inline int get_boot_cpu_id(void)
{
	return __boot_cpu_id;
}

#define smp_processor_id() raw_smp_processor_id()

/*
 * Final polishing of CPUs
 */
extern void smp_cpus_done(unsigned int max_cpus);

/*
 * Bring a CPU up
 */
extern int __cpu_up(unsigned int cpunum, struct task_struct *tidle);

/*
 * stops all CPUs but the current one:
 */
extern void smp_send_stop(void);

/*
 * Prepare machine for booting other CPUs.
 */
extern void smp_prepare_cpus(unsigned int max_cpus);

/*
 * Mark the boot cpu "online" so that it can call console drivers in
 * printk() and can access its per-cpu storage.
 */
void smp_prepare_boot_cpu(void);

/*
 * Call a function on all processors
 */
extern int on_each_cpu(int (*func) (void *info), void *info, int wait);

#endif /* __RTOCHIUS_SMP_H_ */
