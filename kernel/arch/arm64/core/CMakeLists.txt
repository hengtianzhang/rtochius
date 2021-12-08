# SPDX-License-Identifier: Apache-2.0

kernel_library()

kernel_library_sources(
	head.S entry.S traps.c cpuinfo.c init.c setup.c smp.c extable.c
)

set_property(GLOBAL PROPERTY LINKER_SCRIPT_S "${CMAKE_CURRENT_LIST_DIR}/linker.lds.S")
