# SPDX-License-Identifier: Apache-2.0

kernel_library()

kernel_library_sources(
	cache.S proc.S flush.c mmu.c pageattr.c
	copypage.c pgd.c context.c
)
