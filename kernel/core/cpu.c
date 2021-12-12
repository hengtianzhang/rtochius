/* CPU control.
 * (C) 2001, 2002, 2003, 2004 Rusty Russell
 *
 * This code is licenced under the GPL.
 */
#define pr_fmt(fmt) "cpu: " fmt

#include <base/cache.h>
#include <base/init.h>

#include <rtochius/cpu.h>
#include <rtochius/smp.h>
#include <rtochius/cpumask.h>
#include <rtochius/device.h>

struct cpumask __cpu_possible_mask __read_mostly;
struct cpumask __cpu_present_mask __read_mostly;
struct cpumask __cpu_online_mask __read_mostly;

/*
 * cpu_bit_bitmap[] is a special, "compressed" data structure that
 * represents all NR_CPUS bits binary values of 1<<nr.
 *
 * It is used by cpumask_of() to get a constant address to a CPU
 * mask value that has a single bit set only.
 */

/* cpu_bit_bitmap[0] is empty - so we can back into it */
#define MASK_DECLARE_1(x)	[x+1][0] = (1UL << (x))
#define MASK_DECLARE_2(x)	MASK_DECLARE_1(x), MASK_DECLARE_1(x+1)
#define MASK_DECLARE_4(x)	MASK_DECLARE_2(x), MASK_DECLARE_2(x+2)
#define MASK_DECLARE_8(x)	MASK_DECLARE_4(x), MASK_DECLARE_4(x+4)

const unsigned long cpu_bit_bitmap[BITS_PER_LONG+1][BITS_TO_LONGS(NR_CPUS)] = {

	MASK_DECLARE_8(0),	MASK_DECLARE_8(8),
	MASK_DECLARE_8(16),	MASK_DECLARE_8(24),
#if BITS_PER_LONG > 32
	MASK_DECLARE_8(32),	MASK_DECLARE_8(40),
	MASK_DECLARE_8(48),	MASK_DECLARE_8(56),
#endif
};

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
