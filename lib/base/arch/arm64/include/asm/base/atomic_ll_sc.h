/*
 * Based on arch/arm/include/asm/atomic.h
 *
 * Copyright (C) 1996 Russell King.
 * Copyright (C) 2002 Deep Blue Solutions Ltd.
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
#ifndef __ASM_BASE_ATOMIC_LL_SC_H_
#define __ASM_BASE_ATOMIC_LL_SC_H_

#ifndef __ARM64_IN_ATOMIC_IMPL
#error "please don't include this file directly"
#endif

/*
 * AArch64 UP and SMP safe atomic ops.  We use load exclusive and
 * store exclusive to ensure that these are atomic.  We may loop
 * to ensure that the update happens.
 *
 * NOTE: these functions do *not* follow the PCS and must explicitly
 * save any clobbered registers other than x0 (regardless of return
 * value).  This is achieved through -fcall-saved-* compiler flags for
 * this file, which unfortunately don't work on a per-function basis
 * (the optimize attribute silently ignores these options).
 */
#define ATOMIC_OP(op, asm_op)			\
static inline void atomic_##op(int i, atomic_t *v)		\
{									\
	u64 tmp;						\
	int result;							\
									\
	asm volatile("// atomic_" #op "\n"				\
"	prfm	pstl1strm, %2\n"					\
"1:	ldxr	%w0, %2\n"						\
"	" #asm_op "	%w0, %w0, %w3\n"				\
"	stxr	%w1, %w0, %2\n"						\
"	cbnz	%w1, 1b"						\
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)		\
	: "Ir" (i));							\
}

#define ATOMIC_OP_RETURN(name, mb, acq, rel, cl, op, asm_op)	\
static inline int atomic_##op##_return##name(int i, atomic_t *v)	\
{									\
	u64 tmp;						\
	int result;							\
									\
	asm volatile("// atomic_" #op "_return" #name "\n"		\
"	prfm	pstl1strm, %2\n"					\
"1:	ld" #acq "xr	%w0, %2\n"					\
"	" #asm_op "	%w0, %w0, %w3\n"				\
"	st" #rel "xr	%w1, %w0, %2\n"					\
"	cbnz	%w1, 1b\n"						\
"	" #mb								\
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)		\
	: "Ir" (i)							\
	: cl);								\
									\
	return result;							\
}

#define ATOMIC_FETCH_OP(name, mb, acq, rel, cl, op, asm_op)		\
static inline int atomic_fetch_##op##name(int i, atomic_t *v)	\
{									\
	u64 tmp;						\
	int val, result;						\
									\
	asm volatile("// atomic_fetch_" #op #name "\n"			\
"	prfm	pstl1strm, %3\n"					\
"1:	ld" #acq "xr	%w0, %3\n"					\
"	" #asm_op "	%w1, %w0, %w4\n"				\
"	st" #rel "xr	%w2, %w1, %3\n"					\
"	cbnz	%w2, 1b\n"						\
"	" #mb								\
	: "=&r" (result), "=&r" (val), "=&r" (tmp), "+Q" (v->counter)	\
	: "Ir" (i)							\
	: cl);								\
									\
	return result;							\
}

#define ATOMIC_OPS(...)							\
	ATOMIC_OP(__VA_ARGS__)						\
	ATOMIC_OP_RETURN(        , dmb ish,  , l, "memory", __VA_ARGS__)\
	ATOMIC_OP_RETURN(_relaxed,        ,  ,  ,         , __VA_ARGS__)\
	ATOMIC_OP_RETURN(_acquire,        , a,  , "memory", __VA_ARGS__)\
	ATOMIC_OP_RETURN(_release,        ,  , l, "memory", __VA_ARGS__)\
	ATOMIC_FETCH_OP (        , dmb ish,  , l, "memory", __VA_ARGS__)\
	ATOMIC_FETCH_OP (_relaxed,        ,  ,  ,         , __VA_ARGS__)\
	ATOMIC_FETCH_OP (_acquire,        , a,  , "memory", __VA_ARGS__)\
	ATOMIC_FETCH_OP (_release,        ,  , l, "memory", __VA_ARGS__)

ATOMIC_OPS(add, add)
ATOMIC_OPS(sub, sub)

#undef ATOMIC_OPS
#define ATOMIC_OPS(...)							\
	ATOMIC_OP(__VA_ARGS__)						\
	ATOMIC_FETCH_OP (        , dmb ish,  , l, "memory", __VA_ARGS__)\
	ATOMIC_FETCH_OP (_relaxed,        ,  ,  ,         , __VA_ARGS__)\
	ATOMIC_FETCH_OP (_acquire,        , a,  , "memory", __VA_ARGS__)\
	ATOMIC_FETCH_OP (_release,        ,  , l, "memory", __VA_ARGS__)

ATOMIC_OPS(and, and)
ATOMIC_OPS(andnot, bic)
ATOMIC_OPS(or, orr)
ATOMIC_OPS(xor, eor)

#undef ATOMIC_OPS
#undef ATOMIC_FETCH_OP
#undef ATOMIC_OP_RETURN
#undef ATOMIC_OP

#define ATOMIC64_OP(op, asm_op)						\
static inline void atomic64_##op(s64 i, atomic64_t *v)			\
{									\
	s64 result;							\
	u64 tmp;						\
									\
	asm volatile("// atomic64_" #op "\n"				\
"	prfm	pstl1strm, %2\n"					\
"1:	ldxr	%0, %2\n"						\
"	" #asm_op "	%0, %0, %3\n"					\
"	stxr	%w1, %0, %2\n"						\
"	cbnz	%w1, 1b"						\
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)		\
	: "Ir" (i));							\
}

#define ATOMIC64_OP_RETURN(name, mb, acq, rel, cl, op, asm_op)		\
static inline s64 atomic64_##op##_return##name(s64 i, atomic64_t *v)	\
{									\
	s64 result;							\
	u64 tmp;						\
									\
	asm volatile("// atomic64_" #op "_return" #name "\n"		\
"	prfm	pstl1strm, %2\n"					\
"1:	ld" #acq "xr	%0, %2\n"					\
"	" #asm_op "	%0, %0, %3\n"					\
"	st" #rel "xr	%w1, %0, %2\n"					\
"	cbnz	%w1, 1b\n"						\
"	" #mb								\
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)		\
	: "Ir" (i)							\
	: cl);								\
									\
	return result;							\
}

#define ATOMIC64_FETCH_OP(name, mb, acq, rel, cl, op, asm_op)		\
static inline s64 atomic64_fetch_##op##name(s64 i, atomic64_t *v)	\
{									\
	s64 result, val;						\
	u64 tmp;						\
									\
	asm volatile("// atomic64_fetch_" #op #name "\n"		\
"	prfm	pstl1strm, %3\n"					\
"1:	ld" #acq "xr	%0, %3\n"					\
"	" #asm_op "	%1, %0, %4\n"					\
"	st" #rel "xr	%w2, %1, %3\n"					\
"	cbnz	%w2, 1b\n"						\
"	" #mb								\
	: "=&r" (result), "=&r" (val), "=&r" (tmp), "+Q" (v->counter)	\
	: "Ir" (i)							\
	: cl);								\
									\
	return result;							\
}

#define ATOMIC64_OPS(...)						\
	ATOMIC64_OP(__VA_ARGS__)					\
	ATOMIC64_OP_RETURN(, dmb ish,  , l, "memory", __VA_ARGS__)	\
	ATOMIC64_OP_RETURN(_relaxed,,  ,  ,         , __VA_ARGS__)	\
	ATOMIC64_OP_RETURN(_acquire,, a,  , "memory", __VA_ARGS__)	\
	ATOMIC64_OP_RETURN(_release,,  , l, "memory", __VA_ARGS__)	\
	ATOMIC64_FETCH_OP (, dmb ish,  , l, "memory", __VA_ARGS__)	\
	ATOMIC64_FETCH_OP (_relaxed,,  ,  ,         , __VA_ARGS__)	\
	ATOMIC64_FETCH_OP (_acquire,, a,  , "memory", __VA_ARGS__)	\
	ATOMIC64_FETCH_OP (_release,,  , l, "memory", __VA_ARGS__)

ATOMIC64_OPS(add, add)
ATOMIC64_OPS(sub, sub)

#undef ATOMIC64_OPS
#define ATOMIC64_OPS(...)						\
	ATOMIC64_OP(__VA_ARGS__)					\
	ATOMIC64_FETCH_OP (, dmb ish,  , l, "memory", __VA_ARGS__)	\
	ATOMIC64_FETCH_OP (_relaxed,,  ,  ,         , __VA_ARGS__)	\
	ATOMIC64_FETCH_OP (_acquire,, a,  , "memory", __VA_ARGS__)	\
	ATOMIC64_FETCH_OP (_release,,  , l, "memory", __VA_ARGS__)

ATOMIC64_OPS(and, and)
ATOMIC64_OPS(andnot, bic)
ATOMIC64_OPS(or, orr)
ATOMIC64_OPS(xor, eor)

#undef ATOMIC64_OPS
#undef ATOMIC64_FETCH_OP
#undef ATOMIC64_OP_RETURN
#undef ATOMIC64_OP

static inline s64 atomic64_dec_if_positive(atomic64_t *v)
{
	s64 result;
	u64 tmp;

	asm volatile("// atomic64_dec_if_positive\n"
"	prfm	pstl1strm, %2\n"
"1:	ldxr	%0, %2\n"
"	subs	%0, %0, #1\n"
"	b.lt	2f\n"
"	stlxr	%w1, %0, %2\n"
"	cbnz	%w1, 1b\n"
"	dmb	ish\n"
"2:"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	:
	: "cc", "memory");

	return result;
}

#endif /* !__ASM_BASE_ATOMIC_LL_SC_H_ */
