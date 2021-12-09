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
#include <rtochius/device.h>

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

static DEFINE_PER_CPU(struct device *, cpu_sys_devices);

struct device *get_cpu_device(unsigned cpu)
{
	if (cpu < nr_cpu_ids && cpu_possible(cpu))
		return per_cpu(cpu_sys_devices, cpu);
	else
		return NULL;
}

/*
 * register_cpu - Setup a sysfs device for a CPU.
 * @cpu - cpu->hotpluggable field set to 1 will generate a control file in
 *	  sysfs for this CPU.
 * @num - CPU number to use when creating the device.
 *
 * Initialize and register the CPU device.
 */
int register_cpu(struct cpu *cpu, int num)
{
	cpu->node_id = num;
	memset(&cpu->dev, 0x00, sizeof(struct device));
	cpu->dev.of_node = of_get_cpu_node(num, NULL);

	per_cpu(cpu_sys_devices, num) = &cpu->dev;

	return 0;
}
