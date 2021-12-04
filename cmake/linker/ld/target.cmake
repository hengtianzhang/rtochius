# SPDX-License-Identifier: Apache-2.0

if(DEFINED TOOLCHAIN_HOME)
	# When Toolchain home is defined, then we are cross-compiling, so only look
	# for linker in that path, else we are using host tools.
	set(LD_SEARCH_PATH PATHS ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
endif()

find_program(CMAKE_LINKER ${CROSS_COMPILE}ld.bfd ${LD_SEARCH_PATH})
if(NOT CMAKE_LINKER)
	find_program(CMAKE_LINKER ${CROSS_COMPILE}ld ${LD_SEARCH_PATH})
endif()

set_ifndef(LINKERFLAGPREFIX -Wl)

set_property(GLOBAL PROPERTY TOPT "-Wl,-T")

# Run $LINKER_SCRIPT file through the C preprocessor, producing ${linker_script_gen}
# NOTE: ${linker_script_gen} will be produced at build-time; not at configure-time
macro(configure_linker_script linker_script_gen linker_pass_define)
	set(extra_dependencies ${ARGN})

	# Different generators deal with depfiles differently.
	if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
		# Note that the IMPLICIT_DEPENDS option is currently supported only
		# for Makefile generators and will be ignored by other generators.
		set(linker_script_dep IMPLICIT_DEPENDS C ${LINKER_SCRIPT})
	elseif(CMAKE_GENERATOR STREQUAL "Ninja")
		# Using DEPFILE with other generators than Ninja is an error.
		set(linker_script_dep DEPFILE ${linker_script_gen}.dep)
	else()
		# TODO: How would the linker script dependencies work for non-linker
		# script generators.
		message(STATUS "Warning; this generator is not well supported. The
 Linker script may not be regenerated when it should.")
		set(linker_script_dep "")
	endif()

	kernel_get_include_directories_for_lang(C current_includes)
	kernel_get_imported_include_directories_for_lang(C current_imported_includes)
	get_property(current_defines GLOBAL PROPERTY PROPERTY_LINKER_SCRIPT_DEFINES)

	add_custom_command(
		OUTPUT ${linker_script_gen}
		DEPENDS
		${LINKER_SCRIPT}
		${extra_dependencies}
		# NB: 'linker_script_dep' will use a keyword that ends 'DEPENDS'
		${linker_script_dep}
		COMMAND ${CMAKE_C_COMPILER}
		-x assembler-with-cpp
		-undef
		-MD -MF ${linker_script_gen}.dep -MT ${linker_script_gen}
		-D_LINKER
		-D_ASMLANGUAGE
		${current_includes}
		${current_imported_includes}
		${current_defines}
		${linker_pass_define}
		-E ${LINKER_SCRIPT}
		-P # Prevent generation of debug `#line' directives.
		-o ${linker_script_gen}
		BYPRODUCTS
		${linker_script_gen}.dep
		VERBATIM
		WORKING_DIRECTORY ${APPLICATION_BINARY_DIR}
		COMMAND_EXPAND_LISTS
	)
endmacro()

# Run $LINKER_SCRIPT file through the C preprocessor, producing ${linker_script_gen}
# NOTE: ${linker_script_gen} will be produced at build-time; not at configure-time
macro(configure_uservice_linker_script uservice_linker_script_gen uservice_linker_pass_define)
	set(uservice_extra_dependencies ${ARGN})

	# Different generators deal with depfiles differently.
	if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
		# Note that the IMPLICIT_DEPENDS option is currently supported only
		# for Makefile generators and will be ignored by other generators.
		set(uservice_linker_script_dep IMPLICIT_DEPENDS C ${USERVICE_LINKER_SCRIPT})
	elseif(CMAKE_GENERATOR STREQUAL "Ninja")
		# Using DEPFILE with other generators than Ninja is an error.
		set(uservice_linker_script_dep DEPFILE ${uservice_linker_script_gen}.dep)
	else()
		# TODO: How would the linker script dependencies work for non-linker
		# script generators.
		message(STATUS "Warning; this generator is not well supported. The
	Linker script may not be regenerated when it should.")
		set(uservice_linker_script_dep "")
	endif()

	uservice_get_include_directories_for_lang(C uservice_current_includes)
	get_property(uservice_current_defines GLOBAL PROPERTY PROPERTY_LINKER_SCRIPT_DEFINES)

	add_custom_command(
		OUTPUT ${uservice_linker_script_gen}
		DEPENDS
		${USERVICE_LINKER_SCRIPT}
		${extra_dependencies}
		# NB: 'uservice_linker_script_dep' will use a keyword that ends 'DEPENDS'
		${uservice_linker_script_dep}
		COMMAND ${CMAKE_C_COMPILER}
		-x assembler-with-cpp
		-undef
		-MD -MF ${uservice_linker_script_gen}.dep -MT ${uservice_linker_script_gen}
		-D_LINKER
		-D_ASMLANGUAGE
		${uservice_current_includes}
		${uservice_current_defines}
		${uservice_linker_pass_define}
		-E ${USERVICE_LINKER_SCRIPT}
		-P # Prevent generation of debug `#line' directives.
		-o ${uservice_linker_script_gen}
		BYPRODUCTS
		${uservice_linker_script_gen}.dep
		VERBATIM
		WORKING_DIRECTORY ${APPLICATION_BINARY_DIR}
		COMMAND_EXPAND_LISTS
	)
endmacro()

# Force symbols to be entered in the output file as undefined symbols
function(toolchain_ld_kernel_force_undefined_symbols location)
	foreach(symbol ${ARGN})
		kernel_link_libraries(${LINKERFLAGPREFIX},-u,${symbol} ${location})
	endforeach()
endfunction()

function(toolchain_ld_link_kernel_elf)
	cmake_parse_arguments(
		TOOLCHAIN_LD_LINK_ELF                                     # prefix of output variables
		""                                                        # list of names of the boolean arguments
		"TARGET_ELF;OUTPUT_MAP;LINKER_SCRIPT"                     # list of names of scalar arguments
		"DEPENDENCIES" # list of names of list arguments
		${ARGN}                                                   # input args to parse
	)

	if(${CMAKE_LINKER} STREQUAL "${CROSS_COMPILE}ld.bfd")
    	# ld.bfd was found so let's explicitly use that for linking, see #32237
    	set(use_linker "-fuse-ld=bfd")
  	endif()

	target_link_libraries(
		${TOOLCHAIN_LD_LINK_ELF_TARGET_ELF}
		${use_linker}
		${TOPT}
		${TOOLCHAIN_LD_LINK_ELF_LINKER_SCRIPT}
		${LINKERFLAGPREFIX},-Map=${TOOLCHAIN_LD_LINK_ELF_OUTPUT_MAP}
		${LINKERFLAGPREFIX},--whole-archive
		${KERNEL_BUILT_IN_LIBS_PROPERTY}
		version
		${LINKERFLAGPREFIX},--no-whole-archive
		${LINKERFLAGPREFIX},--start-group
		${KERNEL_INTERFACE_LIBS_PROPERTY}
		${LINKERFLAGPREFIX},--end-group
		-nostdlib -static
		${TOOLCHAIN_LD_LINK_ELF_DEPENDENCIES}
	)
endfunction()
