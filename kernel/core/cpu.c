/* CPU control.
 * (C) 2001, 2002, 2003, 2004 Rusty Russell
 *
 * This code is licenced under the GPL.
 */
#include <base/cache.h>
#include <base/init.h>

#include <rtochius/cpu.h>
#include <rtochius/smp.h>
#include <rtochius/cpumask.h>

struct cpumask __cpu_possible_mask __read_mostly;
struct cpumask __cpu_present_mask __read_mostly;
struct cpumask __cpu_online_mask __read_mostly;

int __boot_cpu_id;

/*
 * Activate the first processor.
 */
void __init boot_cpu_init(void)
{
	int cpu = smp_processor_id();

	reset_cpu_all_mask();

	/* Mark the boot cpu "present", "online" etc for SMP and UP case */
	set_cpu_possible(cpu, true);
	set_cpu_present(cpu, true);
	set_cpu_online(cpu, true);

	__boot_cpu_id = cpu;
}
