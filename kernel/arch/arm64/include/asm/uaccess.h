/*
 * Based on arch/arm/include/asm/uaccess.h
 *
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
#ifndef __ASM_UACCESS_H_
#define __ASM_UACCESS_H_

/*
 * User space memory access functions
 */

#include <base/bitops.h>
#include <base/string.h>

#include <rtochius/thread_info.h>

#include <asm/kernel-pgtable.h>
#include <asm/sysreg.h>
#include <asm/ptrace.h>
#include <asm/memory.h>
#include <asm/extable.h>

#define get_ds()	(KERNEL_DS)
#define get_fs()	(current_thread_info()->addr_limit)

static inline void set_fs(mm_segment_t fs)
{
	current_thread_info()->addr_limit = fs;

	/*
	 * Prevent a mispredicted conditional call to set_fs from forwarding
	 * the wrong address limit to access_ok under speculation.
	 */
	spec_bar();

	/* On user-mode return, check fs is correct */
	set_thread_flag(TIF_FSCHECK);

	/*
	 * Enable/disable UAO so that copy_to_user() etc can access
	 * kernel memory with the unprivileged instructions.
	 */
	asm("nop");
}

#define segment_eq(a, b)	((a) == (b))

/*
 * Test whether a block of memory is a valid user space address.
 * Returns 1 if the range is valid, 0 otherwise.
 *
 * This is equivalent to the following test:
 * (u65)addr + (u65)size <= (u65)current->addr_limit + 1
 */
static inline unsigned long __range_ok(const void __user *addr, unsigned long size)
{
	unsigned long ret, limit = current_thread_info()->addr_limit;

	asm volatile(
	// A + B <= C + 1 for all A,B,C, in four easy steps:
	// 1: X = A + B; X' = X % 2^64
	"	adds	%0, %3, %2\n"
	// 2: Set C = 0 if X > 2^64, to guarantee X' > C in step 4
	"	csel	%1, xzr, %1, hi\n"
	// 3: Set X' = ~0 if X >= 2^64. For X == 2^64, this decrements X'
	//    to compensate for the carry flag being set in step 4. For
	//    X > 2^64, X' merely has to remain nonzero, which it does.
	"	csinv	%0, %0, xzr, cc\n"
	// 4: For X < 2^64, this gives us X' - C - 1 <= 0, where the -1
	//    comes from the carry in being clear. Otherwise, we are
	//    testing X' - C == 0, subject to the previous adjustments.
	"	sbcs	xzr, %0, %1\n"
	"	cset	%0, ls\n"
	: "=&r" (ret), "+r" (limit) : "Ir" (size), "0" (addr) : "cc");

	return ret;
}

#define access_ok(addr, size)	__range_ok(addr, size)
#define user_addr_max			get_fs

#define _ASM_EXTABLE(from, to)						\
	"	.pushsection	__ex_table, \"a\"\n"			\
	"	.align		3\n"					\
	"	.long		(" #from " - .), (" #to " - .)\n"	\
	"	.popsection\n"

/*
 * Sanitise a uaccess pointer such that it becomes NULL if above the
 * current addr_limit.
 */
#define uaccess_mask_ptr(ptr) (__typeof__(ptr))__uaccess_mask_ptr(ptr)
static inline void __user *__uaccess_mask_ptr(const void __user *ptr)
{
	void __user *safe_ptr;

	asm volatile(
	"	bics	xzr, %1, %2\n"
	"	csel	%0, %1, xzr, eq\n"
	: "=&r" (safe_ptr)
	: "r" (ptr), "r" (current_thread_info()->addr_limit)
	: "cc");

	csdb();
	return safe_ptr;
}

/*
 * The "__xxx" versions of the user access functions do not verify the address
 * space - it must have been done previously with a separate "access_ok()"
 * call.
 *
 * The "__xxx_error" versions set the third argument to -EFAULT if an error
 * occurs, and leave it unchanged on success.
 */
#define __get_user_asm(instr, alt_instr, reg, x, addr, err)	\
	asm volatile(							\
	"1:"instr "     " reg "1, [%2]\n"		\
	"2:\n"								\
	"	.section .fixup, \"ax\"\n"				\
	"	.align	2\n"						\
	"3:	mov	%w0, %3\n"					\
	"	mov	%1, #0\n"					\
	"	b	2b\n"						\
	"	.previous\n"						\
	_ASM_EXTABLE(1b, 3b)						\
	: "+r" (err), "=&r" (x)						\
	: "r" (addr), "i" (-EFAULT))

#define __get_user_err(x, ptr, err)					\
do {									\
	unsigned long __gu_val;						\
	switch (sizeof(*(ptr))) {					\
	case 1:								\
		__get_user_asm("ldrb", "ldtrb", "%w", __gu_val, (ptr),  \
			       (err));			\
		break;							\
	case 2:								\
		__get_user_asm("ldrh", "ldtrh", "%w", __gu_val, (ptr),  \
			       (err));			\
		break;							\
	case 4:								\
		__get_user_asm("ldr", "ldtr", "%w", __gu_val, (ptr),	\
			       (err));			\
		break;							\
	case 8:								\
		__get_user_asm("ldr", "ldtr", "%x",  __gu_val, (ptr),	\
			       (err));			\
		break;							\
	default:							\
		BUILD_BUG();						\
	}								\
	(x) = (__typeof__(*(ptr)))__gu_val;			\
} while (0)

#define __get_user_check(x, ptr, err)					\
({									\
	__typeof__(*(ptr)) __user *__p = (ptr);				\
	if (access_ok(__p, sizeof(*__p))) {				\
		__p = uaccess_mask_ptr(__p);				\
		__get_user_err((x), __p, (err));			\
	} else {							\
		(x) = 0; (err) = -EFAULT;				\
	}								\
})

#define __get_user_error(x, ptr, err)					\
({									\
	__get_user_check((x), (ptr), (err));				\
	(void)0;							\
})

#define __get_user(x, ptr)						\
({									\
	int __gu_err = 0;						\
	__get_user_check((x), (ptr), __gu_err);				\
	__gu_err;							\
})

#define get_user	__get_user

#define __put_user_asm(instr, alt_instr, reg, x, addr, err)	\
	asm volatile(							\
	"1:"instr "     " reg "1, [%2]\n"		\
	"2:\n"								\
	"	.section .fixup,\"ax\"\n"				\
	"	.align	2\n"						\
	"3:	mov	%w0, %3\n"					\
	"	b	2b\n"						\
	"	.previous\n"						\
	_ASM_EXTABLE(1b, 3b)						\
	: "+r" (err)							\
	: "r" (x), "r" (addr), "i" (-EFAULT))

#define __put_user_err(x, ptr, err)					\
do {									\
	__typeof__(*(ptr)) __pu_val = (x);				\
	switch (sizeof(*(ptr))) {					\
	case 1:								\
		__put_user_asm("strb", "sttrb", "%w", __pu_val, (ptr),	\
			       (err));			\
		break;							\
	case 2:								\
		__put_user_asm("strh", "sttrh", "%w", __pu_val, (ptr),	\
			       (err));			\
		break;							\
	case 4:								\
		__put_user_asm("str", "sttr", "%w", __pu_val, (ptr),	\
			       (err));			\
		break;							\
	case 8:								\
		__put_user_asm("str", "sttr", "%x", __pu_val, (ptr),	\
			       (err));			\
		break;							\
	default:							\
		BUILD_BUG();						\
	}								\
} while (0)

#define __put_user_check(x, ptr, err)					\
({									\
	__typeof__(*(ptr)) __user *__p = (ptr);				\
	if (access_ok(__p, sizeof(*__p))) {				\
		__p = uaccess_mask_ptr(__p);				\
		__put_user_err((x), __p, (err));			\
	} else	{							\
		(err) = -EFAULT;					\
	}								\
})

#define __put_user_error(x, ptr, err)					\
({									\
	__put_user_check((x), (ptr), (err));				\
	(void)0;							\
})

#define __put_user(x, ptr)						\
({									\
	int __pu_err = 0;						\
	__put_user_check((x), (ptr), __pu_err);				\
	__pu_err;							\
})

#define put_user	__put_user

extern unsigned long __arch_copy_from_user(void *to, const void __user *from, unsigned long n);
#define raw_copy_from_user(to, from, n)					\
({									\
	__arch_copy_from_user((to), __uaccess_mask_ptr(from), (n));	\
})

extern unsigned long __arch_copy_to_user(void __user *to, const void *from, unsigned long n);
#define raw_copy_to_user(to, from, n)					\
({									\
	__arch_copy_to_user(__uaccess_mask_ptr(to), (from), (n));	\
})

extern unsigned long __arch_copy_in_user(void __user *to, const void __user *from, unsigned long n);
#define raw_copy_in_user(to, from, n)					\
({									\
	__arch_copy_in_user(__uaccess_mask_ptr(to),			\
			    __uaccess_mask_ptr(from), (n));		\
})

#define INLINE_COPY_TO_USER
#define INLINE_COPY_FROM_USER

extern unsigned long __arch_clear_user(void __user *to, unsigned long n);
static inline unsigned long __clear_user(void __user *to, unsigned long n)
{
	if (access_ok(to, n))
		n = __arch_clear_user(__uaccess_mask_ptr(to), n);
	return n;
}
#define clear_user	__clear_user

extern long strncpy_from_user(char *dest, const char __user *src, long count);

extern long strnlen_user(const char __user *str, long n);

extern void memcpy_flushcache(void *dst, const void *src, size_t cnt);
struct page;
void memcpy_page_flushcache(char *to, struct page *page, size_t offset, size_t len);
extern unsigned long __copy_user_flushcache(void *to, const void __user *from, unsigned long n);

static inline int __copy_from_user_flushcache(void *dst, const void __user *src, unsigned size)
{
	return __copy_user_flushcache(dst, __uaccess_mask_ptr(src), size);
}

#endif /* !__ASM_UACCESS_H_ */
