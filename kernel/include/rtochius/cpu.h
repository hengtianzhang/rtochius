/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/linux/cpu.h - generic cpu definition
 *
 * This is mainly for topological representation. We define the 
 * basic 'struct cpu' here, which can be embedded in per-arch 
 * definitions of processors.
 *
 * Basic handling of the devices is done in drivers/base/cpu.c
 *
 * CPUs are exported via sysfs in the devices/system/cpu
 * directory. 
 */
#ifndef __RTOCHIUS_CPU_H_
#define __RTOCHIUS_CPU_H_

#include <base/types.h>

#include <rtochius/of.h>

struct device;

struct cpu {
	int node_id;		/* The node which contains the CPU */
	struct device dev;
};

extern void boot_cpu_init(void);

extern bool arch_match_cpu_phys_id(int cpu, u64 phys_id);
extern bool arch_find_n_match_cpu_physical_id(struct device_node *cpun,
					      int cpu, unsigned int *thread);

extern struct device *get_cpu_device(unsigned cpu);
extern int register_cpu(struct cpu *cpu, int num);

#endif /* !__RTOCHIUS_CPU_H_ */
