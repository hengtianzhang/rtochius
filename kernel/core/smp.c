/*
 * Generic helpers for smp ipi calls
 *
 * (C) Jens Axboe <jens.axboe@oracle.com> 2008
 */
#define pr_fmt(fmt) "smp: " fmt

#include <base/cache.h>

#include <rtochius/cpumask.h>
#include <rtochius/smp.h>

/* Setup number of possible processor ids */
unsigned int nr_cpu_ids __read_mostly = NR_CPUS;
