/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_BIT_SPINLOCK_H_
#define __RTOCHIUS_BIT_SPINLOCK_H_

#include <base/compiler.h>
#include <base/common.h>
#include <base/bug.h>
#include <base/bitops.h>

#include <rtochius/preempt.h>
#include <rtochius/spinlock.h>

/*
 *  bit-based spin_lock()
 *
 * Don't use this unless you really need to: spin_lock() and spin_unlock()
 * are significantly faster.
 */
static inline void bit_spin_lock(int bitnum, unsigned long *addr)
{
	/*
	 * Assuming the lock is uncontended, this never enters
	 * the body of the outer loop. If it is contended, then
	 * within the inner loop a non-atomic test is used to
	 * busywait with less bus contention for a good time to
	 * attempt to acquire the lock bit.
	 */
	preempt_disable();
	while (unlikely(test_and_set_bit_lock(bitnum, addr))) {
		preempt_enable();
		do {
			cpu_relax();
		} while (test_bit(bitnum, addr));
		preempt_disable();
	}
}

/*
 * Return true if it was acquired
 */
static inline int bit_spin_trylock(int bitnum, unsigned long *addr)
{
	preempt_disable();

	if (unlikely(test_and_set_bit_lock(bitnum, addr))) {
		preempt_enable();
		return 0;
	}

	return 1;
}

/*
 *  bit-based spin_unlock()
 */
static inline void bit_spin_unlock(int bitnum, unsigned long *addr)
{
	clear_bit_unlock(bitnum, addr);

	preempt_enable();
}

/*
 *  bit-based spin_unlock()
 *  non-atomic version, which can be used eg. if the bit lock itself is
 *  protecting the rest of the flags in the word.
 */
static inline void __bit_spin_unlock(int bitnum, unsigned long *addr)
{
	__clear_bit_unlock(bitnum, addr);

	preempt_enable();
}

/*
 * Return true if the lock is held.
 */
static inline int bit_spin_is_locked(int bitnum, unsigned long *addr)
{
	return test_bit(bitnum, addr);
}

#endif /* !__RTOCHIUS_BIT_SPINLOCK_H_ */
