/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __BASE_ERRNAME_H_
#define __BASE_ERRNAME_H_

#include <base/stddef.h>

#ifdef CONFIG_SYMBOLIC_ERRNAME
const char *errname(int err);
#else
static inline const char *errname(int err)
{
	return NULL;
}
#endif

#endif /* !__BASE_ERRNAME_H_ */
