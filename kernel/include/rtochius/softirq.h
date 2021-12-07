#ifndef __RTOCHIUS_SOFTIRQ_H_
#define __RTOCHIUS_SOFTIRQ_H_

#include <base/compiler.h>

#include <rtochius/preempt.h>

static __always_inline void __local_bh_disable_ip(unsigned long ip, unsigned int cnt)
{
	preempt_count_add(cnt);
	barrier();
}

extern void __local_bh_enable_ip(unsigned long ip, unsigned int cnt);

#endif /* !__RTOCHIUS_SOFTIRQ_H_ */
