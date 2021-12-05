/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __BASE_COMPILER_TYPES_H_
#define __BASE_COMPILER_TYPES_H_

#ifndef __ASSEMBLY__

/* Indirect macros required for expanded argument pasting, eg. __LINE__. */
#define ___PASTE(a,b) a##b
#define __PASTE(a,b) ___PASTE(a,b)

/* Attributes */
#include <base/compiler_attributes.h>

/* Compiler specific macros. */
#ifdef __clang__
#include <base/compiler-clang.h>
#elif defined(__GNUC__)
/* The above compilers also define __GNUC__, so order is important here. */
#include <base/compiler-gcc.h>
#else
#error "Unknown compiler"
#endif

/*
 * Some architectures need to provide custom definitions of macros provided
 * by linux/compiler-*.h, and can do so using asm/compiler.h. We include that
 * conditionally rather than using an asm-generic wrapper in order to avoid
 * build failures if any C compilation, which will include this file via an
 * -include argument in c_flags, occurs prior to the asm-generic wrappers being
 * generated.
 */
#ifdef CONFIG_HAVE_ARCH_COMPILER_H
#include <asm/base/compiler.h>
#endif

/*
 * it doesn't make sense on ARM (currently the only user of __naked)
 * to trace naked functions because then mcount is called without
 * stack and frame pointer being set up and there is no chance to
 * restore the lr register to the value before mcount was called.
 */
#define __naked			__attribute__((__naked__)) __attribute__((__no_instrument_function__))

#define __compiler_offsetof(a, b)	__builtin_offsetof(a, b)

#define inline inline __attribute__((__always_inline__)) __gnu_inline \
	__maybe_unused __attribute__((__no_instrument_function__))

#define __inline__ inline
#define __inline   inline

/*
 * Rather then using noinline to prevent stack consumption, use
 * noinline_for_stack instead.  For documentation reasons.
 */
#define noinline_for_stack noinline

#endif /* !__ASSEMBLY__ */

/*
 * The below symbols may be defined for one or more, but not ALL, of the above
 * compilers. We don't consider that to be an error, so set them to nothing.
 * For example, some of them are for compiler specific plugins.
 */
#ifndef __latent_entropy
#define __latent_entropy
#endif

#ifndef __randomize_layout
#define __randomize_layout __designated_init
#endif

#ifndef __no_randomize_layout
#define __no_randomize_layout
#endif

#ifndef randomized_struct_fields_start
#define randomized_struct_fields_start
#define randomized_struct_fields_end
#endif

#ifndef asm_volatile_goto
#define asm_volatile_goto(x...) asm goto(x)
#endif

/* Are two types/vars the same type (ignoring qualifiers)? */
#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

/* Is this type a native word size -- useful for atomic operations */
#define __native_word(t) \
	(sizeof(t) == sizeof(char) || sizeof(t) == sizeof(short) || \
	 sizeof(t) == sizeof(int) || sizeof(t) == sizeof(long long))

/* Helpers for emitting diagnostics in pragmas. */
#ifndef __diag
#define __diag(string)
#endif

#ifndef __diag_GCC
#define __diag_GCC(version, severity, string)
#endif

#define __diag_push()	__diag(push)
#define __diag_pop()	__diag(pop)

#define __diag_ignore(compiler, version, option, comment) \
	__diag_ ## compiler(version, ignore, option)
#define __diag_warn(compiler, version, option, comment) \
	__diag_ ## compiler(version, warn, option)
#define __diag_error(compiler, version, option, comment) \
	__diag_ ## compiler(version, error, option)

#endif /* !__BASE_COMPILER_TYPES_H_ */
