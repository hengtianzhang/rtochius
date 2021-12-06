/*
 * Based on arch/arm/include/asm/cacheflush.h
 *
 * Copyright (C) 1999-2002 Russell King.
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_CACHEFLUSH_H_
#define __ASM_CACHEFLUSH_H_

extern void __flush_icache_range(unsigned long start, unsigned long end);

/*
 *	invalidate_icache_range(start, end)
 *
 *		Invalidate the I-cache in the region described by start, end.
 *		- start  - virtual start address
 *		- end    - virtual end address
 */
extern int invalidate_icache_range(unsigned long start, unsigned long end);

/*
 *	__flush_dcache_area(kaddr, size)
 *
 *		Ensure that the data held in page is written back.
 *		- kaddr  - page address
 *		- size   - region size
 */
extern void __flush_dcache_area(void *addr, size_t len);
extern void __inval_dcache_area(void *addr, size_t len);

extern void __clean_dcache_area_poc(void *addr, size_t len);
extern void __clean_dcache_area_pop(void *addr, size_t len);
extern void __clean_dcache_area_pou(void *addr, size_t len);

/*
 *	__flush_cache_user_range(start, end)
 *
 *		Ensure coherency between the I-cache and the D-cache in the
 *		region described by start, end.
 *		- start  - virtual start address
 *		- end    - virtual end address
 */
extern long __flush_cache_user_range(unsigned long start, unsigned long end);

extern void __dma_clean_area(void *addr, size_t len);
extern void __dma_inv_area(void *addr, size_t len);

/*
 * Cache maintenance functions used by the DMA API. No to be used directly.
 */
extern void __dma_map_area(const void *, size_t, int);
extern void __dma_unmap_area(const void *, size_t, int);
extern void __dma_flush_area(const void *, size_t);

#endif /* !__ASM_CACHEFLUSH_H_ */
