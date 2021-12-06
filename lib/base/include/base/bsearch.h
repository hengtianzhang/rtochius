/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __BASE_BSEARCH_H_
#define __BASE_BSEARCH_H_

#include <base/types.h>

void *bsearch(const void *key, const void *base, size_t num, size_t size,
	      int (*cmp)(const void *key, const void *elt));

#endif /* !__BASE_BSEARCH_H_ */
