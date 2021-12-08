/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Written by Mark Hemment, 1996 (markhe@nextd.demon.co.uk).
 *
 * (C) SGI 2006, Christoph Lameter
 * 	Cleaned up and restructured to ease the addition of alternative
 * 	implementations of SLAB allocators.
 * (C) Linux Foundation 2008-2013
 *      Unified interface for all slab allocators
 */

#ifndef __RTOCHIUS_SLAB_H_
#define	__RTOCHIUS_SLAB_H_

#include <rtochius/gfp.h>

/* Panic if kmem_cache_create() fails */
#define SLAB_PANIC		((slab_flags_t)0x00040000U)

struct kmem_cache {
	int a;
};

static inline struct kmem_cache *kmem_cache_create(const char *name, unsigned int size,
			unsigned int align, slab_flags_t flags,
			void (*ctor)(void *))
{
	return NULL;
}

static inline void *kmem_cache_alloc(struct kmem_cache *a, gfp_t flags)
{
	return NULL;
}
static inline  void kmem_cache_free(struct kmem_cache *a, void *b)
{

}

#endif /* !__RTOCHIUS_SLAB_H_ */
