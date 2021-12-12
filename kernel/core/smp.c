/*
 * Generic helpers for smp ipi calls
 *
 * (C) Jens Axboe <jens.axboe@oracle.com> 2008
 */
#define pr_fmt(fmt) "smp: " fmt

#include <base/cache.h>
#include <base/init.h>

#include <rtochius/cpumask.h>
#include <rtochius/smp.h>

/* Setup number of possible processor ids */
unsigned int nr_cpu_ids __read_mostly = NR_CPUS;

/* An arch may set nr_cpu_ids earlier if needed, so this would be redundant */
void __init setup_nr_cpu_ids(void)
{
	nr_cpu_ids = find_last_bit(cpumask_bits(cpu_possible_mask), NR_CPUS) + 1;
}

/*
 * Call a function on all processors
 */
int on_each_cpu(int (*func) (void *info), void *info, int wait)
{
	return 0;
}
