#ifndef __RTOCHIUS_SPINLOCK_TYPES_H_
#define __RTOCHIUS_SPINLOCK_TYPES_H_

/*
 * include/linux/spinlock_types.h - generic spinlock type definitions
 *                                  and initializers
 *
 * portions Copyright 2005, Red Hat, Inc., Ingo Molnar
 * Released under the General Public License (GPL).
 */
#include <asm/spinlock_types.h>

#include <rtochius/lockdep.h>

typedef struct raw_spinlock {
	arch_spinlock_t raw_lock;
} raw_spinlock_t;

#define SPINLOCK_MAGIC		0xdead4ead

#define SPINLOCK_OWNER_INIT	((void *)-1L)

#define SPIN_DEP_MAP_INIT(lockname)

#define SPIN_DEBUG_INIT(lockname)

#define __RAW_SPIN_LOCK_INITIALIZER(lockname)	\
	{					\
	.raw_lock = __ARCH_SPIN_LOCK_UNLOCKED,	\
	SPIN_DEBUG_INIT(lockname)		\
	SPIN_DEP_MAP_INIT(lockname) }

#define __RAW_SPIN_LOCK_UNLOCKED(lockname)	\
	(raw_spinlock_t) __RAW_SPIN_LOCK_INITIALIZER(lockname)

#define DEFINE_RAW_SPINLOCK(x)	raw_spinlock_t x = __RAW_SPIN_LOCK_UNLOCKED(x)

typedef struct spinlock {
	union {
		struct raw_spinlock rlock;
	};
} spinlock_t;

#define __SPIN_LOCK_INITIALIZER(lockname) \
	{ { .rlock = __RAW_SPIN_LOCK_INITIALIZER(lockname) } }

#define __SPIN_LOCK_UNLOCKED(lockname) \
	(spinlock_t ) __SPIN_LOCK_INITIALIZER(lockname)

#define DEFINE_SPINLOCK(x)	spinlock_t x = __SPIN_LOCK_UNLOCKED(x)

#include <rtochius/rwlock_types.h>

#endif /* !__RTOCHIUS_SPINLOCK_TYPES_H_ */
