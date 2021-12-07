#ifndef __RTOCHIUS_MM_TYPES_H_
#define __RTOCHIUS_MM_TYPES_H_

#include <asm/mmu.h>

struct page {
	unsigned long flags;		/* Atomic flags, some possibly
					 * updated asynchronously */
};

struct mm_struct {
	/* Architecture-specific MM context */
	mm_context_t context;
};

struct vm_area_struct {
	struct mm_struct *vm_mm;	/* The address space we belong to. */
};

#endif /* !__RTOCHIUS_MM_TYPES_H_ */
