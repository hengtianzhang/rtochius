/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SPINLOCK_H_
#define __RTOCHIUS_SPINLOCK_H_

#define __lockfunc __attribute__((section(".spinlock.text")))

/* Linker adds these: start and end of __lockfunc functions */
extern char __lock_text_start[], __lock_text_end[];

#endif /* !__RTOCHIUS_SPINLOCK_H_ */
