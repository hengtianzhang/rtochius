/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SMP_H_
#define __RTOCHIUS_SMP_H_

/*
 *	Generic SMP support
 *		Alan Cox. <alan@redhat.com>
 */
#include <asm/smp.h>

extern void smp_setup_processor_id(void);

extern int __boot_cpu_id;

static inline int get_boot_cpu_id(void)
{
	return __boot_cpu_id;
}

#define smp_processor_id() raw_smp_processor_id()

#endif /* __RTOCHIUS_SMP_H_ */
