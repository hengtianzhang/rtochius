# SPDX-License-Identifier: Apache-2.0

if(NOT DEFINED USERVICE_BINARY_DIR)
	message(FATAL_ERROR "A user error has occurred.
cmake was invoked with '${CMAKE_CURRENT_LIST_DIR}' specified as the source directory,
but it must be invoked with an application source directory,
such as '${CMAKE_CURRENT_LIST_DIR}/XXXX/build/uservice'.
Debug variables:
CMAKE_CACHEFILE_DIR: ${CMAKE_CACHEFILE_DIR}
")
endif()

if(NOT DEFINED USERVICE_SOURCE_DIR)
	message(FATAL_ERROR "A user error has occurred.
cmake was invoked with '${CMAKE_CURRENT_LIST_DIR}' specified as the source directory,
but it must be invoked with an application source directory,
such as '${CMAKE_CURRENT_LIST_DIR}/uservice'.
Debug variables:
CMAKE_CACHEFILE_DIR: ${CMAKE_CACHEFILE_DIR}
")
endif()

# See https://gitlab.kitware.com/cmake/cmake/issues/16228
# and https://cmake.org/pipermail/cmake/2019-May/thread.html#69496
if(NOT ${RTOCHIUS_BASE}/uservice STREQUAL CMAKE_CURRENT_SOURCE_DIR)
message(WARNING "projects doesn't match CMAKE_CURRENT_SOURCE_DIR
    RTOCHIUS_BASE              = ${RTOCHIUS_BASE}
    PWD                      = $ENV{PWD}
    CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}
You may be using a mix of symbolic links and real paths which causes \
subtle and hard to debug CMake issues.")
endif()

# Verify that the toolchain can compile a dummy file, if it is not we
# won't be able to test for compatibility with certain C flags.
rtochius_check_compiler_flag(C "" toolchain_is_ok)
assert(toolchain_is_ok "The toolchain is unable to build a dummy C file. See CMakeError.log.")

project(rootUservice VERSION ${RTOCHIUS_PROJECT_VERSION})
enable_language(C CXX ASM)

add_library(uservice_interface INTERFACE)

uservice_include_directories(
	${APPLICATION_BINARY_DIR}/include
)

uservice_compile_definitions(
	__USERVICE__
)

# Common toolchain-agnostic assembly flags
uservice_compile_options(
	$<$<COMPILE_LANGUAGE:ASM>:-D__ASSEMBLY__>
)

if(BUILD_VERSION)
	uservice_compile_definitions(
		BUILD_VERSION=${BUILD_VERSION}
	)
endif()

# @Intent: Add the basic toolchain warning flags
uservice_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,app_warning_base>>)
uservice_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,app_warning_base>>)

uservice_compile_options($<$<COMPILE_LANGUAGE:C>:-fno-strict-aliasing>)

# @Intent: Add the basic toolchain warning flags
uservice_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,warning_base>>)
uservice_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,warning_base>>)

# Now, warning_extended has initializer overrides prior initialization of this subobject
# To remove it for uservice.
uservice_cc_option(-Wno-initializer-overrides)

get_property(OPTIMIZE_FOR_NO_OPTIMIZATIONS_FLAG TARGET compiler PROPERTY no_optimization)
get_property(OPTIMIZE_FOR_DEBUG_FLAG TARGET compiler PROPERTY optimization_debug)
get_property(OPTIMIZE_FOR_SPEED_FLAG TARGET compiler PROPERTY optimization_speed)
get_property(OPTIMIZE_FOR_SIZE_FLAG  TARGET compiler PROPERTY optimization_size)

# From kconfig choice, pick the actual OPTIMIZATION_FLAG to use.
# Kconfig choice ensures only one of these CONFIG_*_OPTIMIZATIONS is set.
if(CONFIG_NO_OPTIMIZATIONS)
	set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_NO_OPTIMIZATIONS_FLAG})
elseif(CONFIG_DEBUG_OPTIMIZATIONS)
	set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_DEBUG_FLAG})
elseif(CONFIG_SPEED_OPTIMIZATIONS)
	set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_SPEED_FLAG})
elseif(CONFIG_SIZE_OPTIMIZATIONS)
	set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_SIZE_FLAG}) # Default in kconfig
else()
	assert(0 "Unreachable code. Expected optimization level to have been chosen. See projects/Kconfig")
endif()

uservice_compile_options(${OPTIMIZATION_FLAG})

get_property(OPTIMIZE_DEBUG  TARGET compiler PROPERTY debug)
uservice_compile_options_ifdef(CONFIG_DEBUG_APP ${OPTIMIZE_DEBUG})

uservice_compile_options("SHELL: $<TARGET_PROPERTY:compiler,imacros> ${KCONFIG_H}")

# @Intent: Set compiler specific flag for bare metal freestanding option
uservice_compile_options($<TARGET_PROPERTY:compiler,freestanding>)

uservice_compile_options($<TARGET_PROPERTY:compiler,diagnostic>)

uservice_compile_options($<TARGET_PROPERTY:compiler,nostdinc>)
target_include_directories(uservice_interface SYSTEM INTERFACE $<TARGET_PROPERTY:compiler,nostdinc_include>)

uservice_ld_options(-nostdlib -static)

add_subdirectory(rtochius_runtime)

set_property(GLOBAL PROPERTY USERVICE_ARCHIVE_FILE "${USERVICE_BINARY_DIR}/archive.S")
set_property(GLOBAL PROPERTY DRIVERS_ARCHIVE_FILE "${USERVICE_BINARY_DIR}/drivers_archive.S")
set_property(GLOBAL PROPERTY USERVICE_ARCHIVE_FILE_LIST "")
set_property(GLOBAL PROPERTY DRIVERS_ARCHIVE_FILE_LIST "")

get_property(USERVICE_LINKER_SCRIPT GLOBAL PROPERTY USERVICE_LINKER_SCRIPT_S)
if(NOT EXISTS ${USERVICE_LINKER_SCRIPT})
	message(FATAL_ERROR "Could not find rtochius linker script: '${CMAKE_CURRENT_SOURCE_DIR}'. ${CMAKE_CURRENT_LIST_FILE} Corrupted configuration?")
endif()

find_package(base REQUIRED)
base_user_import_libraries()

uservice_get_raw_include_directories_for_lang(base C USERVICE_IMPORTED_INCLUDE_DIRS)
uservice_get_include_directories_for_lang(C USERVICE_INCLUDE_DIRS STRIP_PREFIX)

configure_uservice_linker_script(
	${USERVICE_BINARY_DIR}/linker.lds
	"-D__ASSEMBLY__;-DLINKER_SCRIPT;-DAPP_LINKER_SCRIPT;-U${ARCH};${USERVICE_IMPORTED_INCLUDE_DIRS}"
)

add_custom_target(
	linker_uservice_prebuilt_script_target
	DEPENDS
	${USERVICE_BINARY_DIR}/linker.lds
)

set_property(TARGET
	linker_uservice_prebuilt_script_target
	PROPERTY INCLUDE_DIRECTORIES
	${USERVICE_INCLUDE_DIRS}
)

find_package(librtochius REQUIRED)
librtochius_user_import_libraries()

function(DeclareUserviceApp userviceappname)
	target_link_libraries(${userviceappname} PRIVATE rtochius_runtime_crt_obj)

    set_property(
        TARGET ${userviceappname}
        APPEND_STRING
        PROPERTY LINK_OPTIONS -Wl,-T ${USERVICE_BINARY_DIR}/linker.lds
    )
	set_property(
		TARGET
		${userviceappname}
		PROPERTY LINK_DEPENDS
		${USERVICE_BINARY_DIR}/linker.lds
	)
	add_dependencies(${userviceappname} linker_uservice_prebuilt_script_target)

	target_link_libraries(${userviceappname} PUBLIC uservice_interface)
	target_link_libraries(${userviceappname} PRIVATE librtochius)

	set_property(GLOBAL APPEND PROPERTY USERVICE_ARCHIVE_FILE_LIST "$<TARGET_FILE:${userviceappname}>")
endfunction(DeclareUserviceApp)

function(DeclareDriversApp driversappname)
	target_link_libraries(${driversappname} PRIVATE rtochius_runtime_crt_obj)

    set_property(
        TARGET ${driversappname}
        APPEND_STRING
        PROPERTY LINK_OPTIONS -Wl,-T ${USERVICE_BINARY_DIR}/linker.lds
    )
	set_property(
		TARGET
		${driversappname}
		PROPERTY LINK_DEPENDS
		${USERVICE_BINARY_DIR}/linker.lds
	)
	add_dependencies(${driversappname} linker_uservice_prebuilt_script_target)

	target_link_libraries(${driversappname} PUBLIC uservice_interface)
	target_link_libraries(${driversappname} PRIVATE librtochius)

	set_property(GLOBAL APPEND PROPERTY DRIVERS_ARCHIVE_FILE_LIST "$<TARGET_FILE:${driversappname}>")
endfunction()

function(make_kernel_uservice_archive output_name)
	MakeCPIO(commands ${output_name} ${ARGN})
	add_custom_command(
		OUTPUT ${output_name}
		COMMAND rm -f ${output_name}
		COMMAND rm -f ${output_name}.cpio
		COMMAND ${commands}
		COMMAND ${SHELL} -c
		"echo 'X.section .archive_uservice,\"aw\"X.globl __archive_uservice_start, \
		__archive_uservice_endX__archive_uservice_start:X.incbin \"${output_name}.cpio\"X__archive_uservice_end:X' | tr X '\\n'"
		> ${output_name}
		BYPRODUCTS
		${output_name}.cpio
		DEPENDS
		${ARGN}
		VERBATIM
	)
	add_custom_target(uservice_archive DEPENDS ${output_name})
endfunction()

function(make_kernel_drivers_archive output_name)
	MakeCPIO(commands ${output_name} ${ARGN})
	add_custom_command(
		OUTPUT ${output_name}
		COMMAND rm -f ${output_name}
		COMMAND rm -f ${output_name}.cpio
		COMMAND ${commands}
		COMMAND ${SHELL} -c
		"echo 'X.section .archive_drivers,\"aw\"X.globl __archive_drivers_start, \
		__archive_drivers_endX__archive_drivers_start:X.incbin \"${output_name}.cpio\"X__archive_drivers_end:X' | tr X '\\n'"
		> ${output_name}
		BYPRODUCTS
		${output_name}.cpio
		DEPENDS
		${ARGN}
		VERBATIM
	)
	add_custom_target(drivers_archive DEPENDS ${output_name})
endfunction()

function(qemu_virt_setup)
	if (CONFIG_QEMU_VIRT)
		if (NOT CONFIG_QEMU_INITRD)
			set(qemu_initrd "none")
		else()
			set(qemu_initrd ${CONFIG_QEMU_INITRD})
		endif()
		get_property(CORE_IMAGE_NAME_PROPERTY GLOBAL PROPERTY CORE_IMAGE_NAME)
		get_property(CORE_IMAGE_TARGET_PROPERTY GLOBAL PROPERTY CORE_IMAGE_TARGET)

		if(CONFIG_QEMU_APPEND)
			set(A_QEMU_APPEND "-a ${CONFIG_QEMU_APPEND}")
		else()
			set(A_QEMU_APPEND "-a none")
		endif()

		if(CONFIG_QEMU_INITRD)
			set(R_QEMU_INITRD "-r ${CONFIG_QEMU_INITRD}")
		else()
			set(R_QEMU_INITRD "-r none")
		endif()

		if(CONFIG_QEMU_GRAPHIC)
			set(G_QEMU_GRAPHIC "-g ${CONFIG_QEMU_GRAPHIC}")
		else()
			set(G_QEMU_GRAPHIC "-g none")
		endif()

		if(CONFIG_QEMU_DEBUG)
			set(D_QEMU_DEBUG "-D ${CONFIG_QEMU_DEBUG}")
		else()
			set(D_QEMU_DEBUG "-D none")
		endif()

		add_custom_command(
			OUTPUT ${APPLICATION_BINARY_DIR}/simulate
			COMMAND ${PYTHON_EXECUTABLE} ${RTOCHIUS_BASE}/scripts/mksimulate.py
			-n ${CORE_IMAGE_NAME_PROPERTY}
			-o ${APPLICATION_BINARY_DIR}/simulate
			-s ${CONFIG_QEMU_SYSTEM}
			-c ${CONFIG_QEMU_CPU}
			-m ${CONFIG_QEMU_MACHINE}
			-p ${CONFIG_QEMU_VIRT_OPTION}
			${A_QEMU_APPEND}
			${R_QEMU_INITRD}
			-M ${CONFIG_QEMU_MEMORY}
			${G_QEMU_GRAPHIC}
			${D_QEMU_DEBUG}
			COMMAND chmod u+x "${APPLICATION_BINARY_DIR}/simulate"
			DEPENDS
			${CORE_IMAGE_TARGET_PROPERTY}
			WORKING_DIRECTORY
			${APPLICATION_BINARY_DIR}
		)
		add_custom_target(qemu_virt ALL DEPENDS ${APPLICATION_BINARY_DIR}/simulate)
	endif()
endfunction()
