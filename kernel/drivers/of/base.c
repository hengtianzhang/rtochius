// SPDX-License-Identifier: GPL-2.0+
/*
 * Procedures for creating, accessing and interpreting the device tree.
 *
 * Paul Mackerras	August 1996.
 * Copyright (C) 1996-2005 Paul Mackerras.
 *
 *  Adapted for 64bit PowerPC by Dave Engebretsen and Peter Bergner.
 *    {engebret|bergner}@us.ibm.com
 *
 *  Adapted for sparc and sparc64 by David S. Miller davem@davemloft.net
 *
 *  Reconsolidated from arch/x/kernel/prom.c by Stephen Rothwell and
 *  Grant Likely.
 */

#define pr_fmt(fmt)	"OF: " fmt

#include <base/types.h>
#include <base/list.h>

#include "of_private.h"

LIST_HEAD(aliases_lookup);
