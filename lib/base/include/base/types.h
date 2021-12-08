#ifndef __BASE_TYPES_H_
#define __BASE_TYPES_H_

#include <base/stddef.h>

#include <asm/base/types.h>

#ifndef __ASSEMBLY__

typedef __s8  	s8;
typedef __u8  	u8;
typedef __s16 	s16;
typedef __u16	u16;
typedef __s32 	s32;
typedef __u32 	u32;
typedef __s64 	s64;
typedef __u64 	u64;

#define S8_C(x)  x
#define U8_C(x)  x ## U
#define S16_C(x) x
#define U16_C(x) x ## U
#define S32_C(x) x
#define U32_C(x) x ## U
#define S64_C(x) x ## LL
#define U64_C(x) x ## ULL

#if __BITS_PER_LONG != 64
typedef u32		size_t;
typedef s32		ssize_t;
#else
typedef unsigned long		size_t;
typedef __signed__ long		ssize_t;
#endif

typedef unsigned short		umode_t;

typedef _Bool	bool;

typedef u8			uint8_t;
typedef u16			uint16_t;
typedef u32			uint32_t;

#if defined(__GNUC__)
typedef u64			uint64_t;
typedef u64			u_int64_t;
typedef s64			int64_t;
#endif

/* this is a special 64bit data type that is 8-byte aligned */
#define aligned_u64		__aligned_u64
#define aligned_be64		__aligned_be64
#define aligned_le64		__aligned_le64

#ifdef CONFIG_ARCH_PHYS_ADDR_T_64BIT
typedef u64 	phys_addr_t;
#else
typedef u32 	phys_addr_t;
#endif

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
typedef u64 	dma_addr_t;
#else
typedef u32 	dma_addr_t;
#endif

typedef s64		ptrdiff_t;

typedef unsigned long		uintptr_t;
typedef int         pid_t;
typedef s64			loff_t;
typedef s64 	suseconds_t;
typedef s64		timer_t;
typedef s32		clockid_t;

typedef s64		time_t;
typedef s64		clock_t;

typedef unsigned __bitwise gfp_t;
typedef unsigned __bitwise slab_flags_t;

typedef int endpoint_t;			/* process identifier */

#else

#define S8_C(x)  x
#define U8_C(x)  x
#define S16_C(x) x
#define U16_C(x) x
#define S32_C(x) x
#define U32_C(x) x
#define S64_C(x) x
#define U64_C(x) x

#endif /* !__ASSEMBLY__ */
#endif /* !__BASE_TYPES_H_ */
