/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SLAB_DEF_H_
#define __RTOCHIUS_SLAB_DEF_H_

#include <base/compiler.h>
#include <base/list.h>
#include <base/log2.h>

#include <rtochius/spinlock.h>

#include <asm/cache.h>

#include <asm/base/page-def.h>

struct page;

struct kmem_cache_cpu {
	void **freelist;
	struct page *page;
	unsigned int offset;
	unsigned int objsize;
};

struct kmem_cache_node {
	spinlock_t list_lock;	/* Protect partial list and nr_partial */
	unsigned long nr_partial;
	atomic_long_t nr_slabs;
	struct list_head partial;
};

/*
 * Slab cache management.
 */
struct kmem_cache {
	/* Used for retriving partial slabs etc */
	unsigned long flags;
	int size;		/* The size of an object including meta data */
	int objsize;		/* The size of an object without meta data */
	int offset;		/* Free pointer offset. */
	int order;

	/*
	 * Avoid an extra cache line for UP, SMP and for the node local to
	 * struct kmem_cache.
	 */
	struct kmem_cache_node local_node;

	/* Allocation and freeing of slabs */
	int objects;		/* Number of objects in slab */
	int refcount;		/* Refcount for slab cache destroy */
	void (*ctor)(struct kmem_cache *, void *);
	int inuse;		/* Offset to metadata */
	int align;		/* Alignment */
	const char *name;	/* Name (only for display!) */
	struct list_head list;	/* List of slab caches */

	struct kmem_cache_cpu *cpu_slab[NR_CPUS];
};

#if defined(ARCH_DMA_MINALIGN) && ARCH_DMA_MINALIGN > 8
#define ARCH_KMALLOC_MINALIGN ARCH_DMA_MINALIGN
#define KMALLOC_MIN_SIZE ARCH_DMA_MINALIGN
#else
#define ARCH_KMALLOC_MINALIGN __alignof__(unsigned long long)
#endif

#define KMALLOC_SHIFT_LOW ilog2(ARCH_KMALLOC_MINALIGN)

/*
 * We keep the general caches in an array of slab caches that are used for
 * 2^x bytes of allocations.
 */
extern struct kmem_cache kmalloc_caches[PAGE_SHIFT];

/*
 * Sorry that the following has to be that ugly but some versions of GCC
 * have trouble with constant propagation and loops.
 */
static __always_inline int kmalloc_index(size_t size)
{
	if (!size)
		return 0;

	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_SHIFT_LOW;

	if (size > 64 && size <= 96)
		return 1;
	if (size > 128 && size <= 192)
		return 2;
	if (size <=          8) return 3;
	if (size <=         16) return 4;
	if (size <=         32) return 5;
	if (size <=         64) return 6;
	if (size <=        128) return 7;
	if (size <=        256) return 8;
	if (size <=        512) return 9;
	if (size <=       1024) return 10;
	if (size <=   2 * 1024) return 11;
/*
 * The following is only needed to support architectures with a larger page
 * size than 4k.
 */
	if (size <=   4 * 1024) return 12;
	if (size <=   8 * 1024) return 13;
	if (size <=  16 * 1024) return 14;
	if (size <=  32 * 1024) return 15;
	if (size <=  64 * 1024) return 16;
	if (size <= 128 * 1024) return 17;
	if (size <= 256 * 1024) return 18;
	if (size <= 512 * 1024) return 19;
	if (size <= 1024 * 1024) return 20;
	if (size <=  2 * 1024 * 1024) return 21;
	return -1;

/*
 * What we really wanted to do and cannot do because of compiler issues is:
 *	int i;
 *	for (i = KMALLOC_SHIFT_LOW; i <= KMALLOC_SHIFT_HIGH; i++)
 *		if (size <= (1 << i))
 *			return i;
 */
}

/*
 * Find the slab cache for a given combination of allocation flags and size.
 *
 * This ought to end up with a global pointer to the right cache
 * in kmalloc_caches.
 */
static __always_inline struct kmem_cache *kmalloc_slab(size_t size)
{
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return &kmalloc_caches[index];
}

#endif /* !__RTOCHIUS_SLAB_DEF_H_ */
