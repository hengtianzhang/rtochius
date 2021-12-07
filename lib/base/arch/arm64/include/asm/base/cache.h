#ifndef __ASM_BASE_CACHE_H_
#define __ASM_BASE_CACHE_H_

#define L1_CACHE_SHIFT		(6)
#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)

/*
 * Memory returned by kmalloc() may be used for DMA, so we must make
 * sure that all such allocations are cache aligned. Otherwise,
 * unrelated code may cause parts of the buffer to be read into the
 * cache before the transfer is done, causing old data to be seen by
 * the CPU.
 */
#define ARCH_DMA_MINALIGN	(128)

#ifndef __ASSEMBLY__

#define __read_mostly __attribute__((__section__(".data..read_mostly")))

#endif /* !__ASSEMBLY__ */
#endif /* !__ASM_BASE_CACHE_H_ */
