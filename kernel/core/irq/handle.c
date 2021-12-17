// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 1992, 1998-2006 Linus Torvalds, Ingo Molnar
 * Copyright (C) 2005-2006, Thomas Gleixner, Russell King
 *
 * This file contains the core interrupt handling code. Detailed
 * information is available in Documentation/core-api/genericirq.rst
 *
 */
#include <base/errno.h>
#include <base/init.h>

#include <rtochius/irq.h>

void (*handle_arch_irq)(struct pt_regs *) __ro_after_init;
