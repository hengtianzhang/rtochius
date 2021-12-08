/*
 * Based on arch/arm/mm/context.c
 *
 * Copyright (C) 2002-2003 Deep Blue Solutions Ltd, all rights reserved.
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
#include <base/types.h>
#include <base/linkage.h>
#include <base/overflow.h>

#include <asm/mmu_context.h>

/* Errata workaround post TTBRx_EL1 update. */
asmlinkage void post_ttbr_update_workaround(void)
{
	asm("nop; nop; nop");
}

void check_and_switch_context(struct mm_struct *mm, unsigned int cpu)
{}

/* Check if the current cpu's ASIDBits is compatible with asid_bits */
void verify_cpu_asid_bits(void)
{
}
