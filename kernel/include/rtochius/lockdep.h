/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Runtime locking correctness validator
 *
 *  Copyright (C) 2006,2007 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *  Copyright (C) 2007 Red Hat, Inc., Peter Zijlstra
 *
 * see Documentation/locking/lockdep-design.txt for more details.
 */
#ifndef __RTOCHIUS_LOCKDEP_H_
#define __RTOCHIUS_LOCKDEP_H_

struct task_struct;
struct lockdep_map;

#define MAX_LOCKDEP_SUBCLASSES		8UL

#include <base/types.h>

static inline void lockdep_off(void)
{
}

static inline void lockdep_on(void)
{
}

# define lock_acquire(l, s, t, r, c, n, i)	do { } while (0)
# define lock_release(l, n, i)			do { } while (0)
# define lock_downgrade(l, i)			do { } while (0)
# define lock_set_class(l, n, k, s, i)		do { } while (0)
# define lock_set_subclass(l, s, i)		do { } while (0)
# define lockdep_init()				do { } while (0)
# define lockdep_init_map(lock, name, key, sub) \
		do { (void)(name); (void)(key); } while (0)
# define lockdep_set_class(lock, key)		do { (void)(key); } while (0)
# define lockdep_set_class_and_name(lock, key, name) \
		do { (void)(key); (void)(name); } while (0)
#define lockdep_set_class_and_subclass(lock, key, sub) \
		do { (void)(key); } while (0)
#define lockdep_set_subclass(lock, sub)		do { } while (0)

#define lockdep_set_novalidate_class(lock) do { } while (0)

/*
 * We don't define lockdep_match_class() and lockdep_match_key() for !LOCKDEP
 * case since the result is not well defined and the caller should rather
 * #ifdef the call himself.
 */

# define lockdep_reset()		do { debug_locks = 1; } while (0)
# define lockdep_free_key_range(start, size)	do { } while (0)
# define lockdep_sys_exit() 			do { } while (0)
/*
 * The class key takes no space if lockdep is disabled:
 */
struct lock_class_key { };

/*
 * The lockdep_map takes no space if lockdep is disabled:
 */
struct lockdep_map { };

#define lockdep_depth(tsk)	(0)

#define lockdep_is_held_type(l, r)		(1)

#define lockdep_assert_held(l)			do { (void)(l); } while (0)
#define lockdep_assert_held_exclusive(l)	do { (void)(l); } while (0)
#define lockdep_assert_held_read(l)		do { (void)(l); } while (0)
#define lockdep_assert_held_once(l)		do { (void)(l); } while (0)

#define lockdep_recursing(tsk)			(0)

struct pin_cookie { };

#define NIL_COOKIE (struct pin_cookie){ }

#define lockdep_pin_lock(l)			({ struct pin_cookie cookie; cookie; })
#define lockdep_repin_lock(l, c)		do { (void)(l); (void)(c); } while (0)
#define lockdep_unpin_lock(l, c)		do { (void)(l); (void)(c); } while (0)


enum xhlock_context_t {
	XHLOCK_HARD,
	XHLOCK_SOFT,
	XHLOCK_CTX_NR,
};

#define lockdep_init_map_crosslock(m, n, k, s) do {} while (0)
/*
 * To initialize a lockdep_map statically use this macro.
 * Note that _name must not be NULL.
 */
#define STATIC_LOCKDEP_MAP_INIT(_name, _key) \
	{ .name = (_name), .key = (void *)(_key), }

static inline void lockdep_invariant_state(bool force) {}
static inline void lockdep_init_task(struct task_struct *task) {}
static inline void lockdep_free_task(struct task_struct *task) {}

#define lock_contended(lockdep_map, ip) do {} while (0)
#define lock_acquired(lockdep_map, ip) do {} while (0)

#define LOCK_CONTENDED(_lock, try, lock) \
	lock(_lock)

#define LOCK_CONTENDED_RETURN(_lock, try, lock) \
	lock(_lock)

#define LOCK_CONTENDED_FLAGS(_lock, try, lock, lockfl, flags) \
	lockfl((_lock), (flags))

static inline void print_irqtrace_events(struct task_struct *curr)
{
}

/*
 * For trivial one-depth nesting of a lock-class, the following
 * global define can be used. (Subsystems with multiple levels
 * of nesting should define their own lock-nesting subclasses.)
 */
#define SINGLE_DEPTH_NESTING			1

/*
 * Map the dependency ops to NOP or to real lockdep ops, depending
 * on the per lock-class debug mode:
 */

#define lock_acquire_exclusive(l, s, t, n, i)		lock_acquire(l, s, t, 0, 1, n, i)
#define lock_acquire_shared(l, s, t, n, i)		lock_acquire(l, s, t, 1, 1, n, i)
#define lock_acquire_shared_recursive(l, s, t, n, i)	lock_acquire(l, s, t, 2, 1, n, i)

#define spin_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)
#define spin_acquire_nest(l, s, t, n, i)	lock_acquire_exclusive(l, s, t, n, i)
#define spin_release(l, n, i)			lock_release(l, n, i)

#define rwlock_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)
#define rwlock_acquire_read(l, s, t, i)		lock_acquire_shared_recursive(l, s, t, NULL, i)
#define rwlock_release(l, n, i)			lock_release(l, n, i)

#define seqcount_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)
#define seqcount_acquire_read(l, s, t, i)	lock_acquire_shared_recursive(l, s, t, NULL, i)
#define seqcount_release(l, n, i)		lock_release(l, n, i)

#define mutex_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)
#define mutex_acquire_nest(l, s, t, n, i)	lock_acquire_exclusive(l, s, t, n, i)
#define mutex_release(l, n, i)			lock_release(l, n, i)

#define rwsem_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)
#define rwsem_acquire_nest(l, s, t, n, i)	lock_acquire_exclusive(l, s, t, n, i)
#define rwsem_acquire_read(l, s, t, i)		lock_acquire_shared(l, s, t, NULL, i)
#define rwsem_release(l, n, i)			lock_release(l, n, i)

#define lock_map_acquire(l)			lock_acquire_exclusive(l, 0, 0, NULL, _THIS_IP_)
#define lock_map_acquire_read(l)		lock_acquire_shared_recursive(l, 0, 0, NULL, _THIS_IP_)
#define lock_map_acquire_tryread(l)		lock_acquire_shared_recursive(l, 0, 1, NULL, _THIS_IP_)
#define lock_map_release(l)			lock_release(l, 1, _THIS_IP_)

# define might_lock(lock) do { } while (0)
# define might_lock_read(lock) do { } while (0)
# define lockdep_assert_irqs_enabled() do { } while (0)
# define lockdep_assert_irqs_disabled() do { } while (0)

static inline void
lockdep_rcu_suspicious(const char *file, const int line, const char *s)
{
}

#endif /* !__RTOCHIUS_LOCKDEP_H_ */
