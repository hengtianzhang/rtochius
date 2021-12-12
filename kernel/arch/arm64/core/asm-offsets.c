/*
 * Copyright (C) 1995-2003 Russell King
 *               2001-2002 Keith Owens
 *     
 * Generate definitions needed by assembly language modules.
 * This code generates raw asm output which is post-processed to extract
 * and format the required data.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <base/compiler.h>

#include <rtochius/mm_types.h>
#include <rtochius/kbuild.h>
#include <rtochius/arm-smccc.h>

#include <asm/smp.h>

int main(void)
{
	BLANK();
	DEFINE(MM_CONTEXT_ID,		offsetof(struct mm_struct, context.id.counter));
	BLANK();
	DEFINE(VMA_VM_MM,		offsetof(struct vm_area_struct, vm_mm));
	BLANK();
	DEFINE(CPU_BOOT_STACK,	offsetof(struct secondary_data, stack));
	DEFINE(CPU_BOOT_TASK,		offsetof(struct secondary_data, task));
	BLANK();
	DEFINE(ARM_SMCCC_RES_X0_OFFS,		offsetof(struct arm_smccc_res, a0));
	DEFINE(ARM_SMCCC_RES_X2_OFFS,		offsetof(struct arm_smccc_res, a2));
	DEFINE(ARM_SMCCC_QUIRK_ID_OFFS,	offsetof(struct arm_smccc_quirk, id));
	DEFINE(ARM_SMCCC_QUIRK_STATE_OFFS,	offsetof(struct arm_smccc_quirk, state));
	BLANK();

	return 0;
}
