/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_DUMP_STACK_H_
#define __RTOCHIUS_DUMP_STACK_H_

#include <base/compiler.h>

__printf(1, 2) void dump_stack_set_arch_desc(const char *fmt, ...);

#endif /* !__RTOCHIUS_DUMP_STACK_H_ */
