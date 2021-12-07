/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_CPUMASK_H_
#define __RTOCHIUS_CPUMASK_H_

#include <rtochius/threads.h>

#if NR_CPUS == 1
#define nr_cpu_ids		1U
#else
extern unsigned int nr_cpu_ids;
#endif

#endif /* !__RTOCHIUS_CPUMASK_H_ */
