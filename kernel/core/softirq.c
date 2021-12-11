/*
 *	linux/kernel/softirq.c
 *
 *	Copyright (C) 1992 Linus Torvalds
 *
 *	Distribute under GPLv2.
 *
 *	Rewritten. Old one was good in 2.2, but in 2.3 it was immoral. --ANK (990903)
 */

#define pr_fmt(fmt) "softirq: " fmt

#include <rtochius/softirq.h>

void __local_bh_enable_ip(unsigned long ip, unsigned int cnt)
{
}
