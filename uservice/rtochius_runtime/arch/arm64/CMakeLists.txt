# SPDX-License-Identifier: Apache-2.0

uservice_ld_options(-Wl,--no-undefined -Wl,-X)
uservice_cc_option(-mabi=lp64)

if(CONFIG_CPU_BIG_ENDIAN)
	uservice_compile_definitions(__AARCH64EB__)
	uservice_compile_options(-mbig-endian)
	target_link_libraries(uservice_interface
		INTERFACE
		${LINKERFLAGPREFIX},-EB
		${LINKERFLAGPREFIX},-maarch64elfb
	)
else()
	uservice_compile_definitions(__AARCH64EL__)
	uservice_compile_options(-mlittle-endian)
	uservice_ld_options(${LINKERFLAGPREFIX},-EL ${LINKERFLAGPREFIX},-maarch64elf)
endif()
