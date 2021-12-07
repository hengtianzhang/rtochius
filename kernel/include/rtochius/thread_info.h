/* SPDX-License-Identifier: GPL-2.0 */
/* thread_info.h: common low-level thread information accessors
 *
 * Copyright (C) 2002  David Howells (dhowells@redhat.com)
 * - Incorporating suggestions made by Linus Torvalds
 */

#ifndef __RTOCHIUS_THREAD_INFO_H_
#define __RTOCHIUS_THREAD_INFO_H_

/*
 * For CONFIG_THREAD_INFO_IN_TASK kernels we need <asm/current.h> for the
 * definition of current, but for !CONFIG_THREAD_INFO_IN_TASK kernels,
 * including <asm/current.h> can cause a circular dependency on some platforms.
 */
#include <asm/current.h>

#define current_thread_info() ((struct thread_info *)current)

#include <asm/thread_info.h>

#endif /* !__RTOCHIUS_THREAD_INFO_H_ */
