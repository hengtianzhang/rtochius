/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_MM_H_
#define __RTOCHIUS_MM_H_

#include <rtochius/page-flags.h>
#include <rtochius/mm_types.h>
#include <rtochius/memory.h>

#ifndef __pa_symbol
#define __pa_symbol(x)  __pa(RELOC_HIDE((unsigned long)(x), 0))
#endif

#ifndef page_to_virt
#define page_to_virt(x)	__va(PFN_PHYS(page_to_pfn(x)))
#endif

#ifndef lm_alias
#define lm_alias(x)	__va(__pa_symbol(x))
#endif

#define page_address(page) page_to_virt(page)

static inline unsigned int compound_order(struct page *page)
{
	return 0;
}

static inline bool pgtable_page_ctor(struct page *page)
{
	return true;
}

static inline void pgtable_page_dtor(struct page *page)
{
}

#endif /* !__RTOCHIUS_MM_H_ */
