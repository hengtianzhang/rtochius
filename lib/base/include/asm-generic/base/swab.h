/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __ASM_GENERIC_BASE_SWAB_H_
#define __ASM_GENERIC_BASE_SWAB_H_

#include <asm/base/types.h>

/*
 * 32 bit architectures typically (but not always) want to
 * set __SWAB_64_THRU_32__. In user space, this is only
 * valid if the compiler supports 64 bit data types.
 */

#if __BITS_PER_LONG == 32
#if defined(__GNUC__) && !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#define __SWAB_64_THRU_32__
#endif
#endif

#endif /* !__ASM_GENERIC_BASE_SWAB_H_ */
