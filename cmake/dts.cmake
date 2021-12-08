# SPDX-License-Identifier: Apache-2.0

function(platform_add_dts_file dtsfile)
	set(file "${CMAKE_CURRENT_SOURCE_DIR}/${dtsfile}")
	if(NOT EXISTS ${file})
		message(WARNING "Not found dts file: ${file}")
		return()
	endif()

	file(RELATIVE_PATH platform_dir_name ${platform_dts_files_root_dir} ${CMAKE_CURRENT_SOURCE_DIR})
	set(platform_dtbs_output_dir ${APPLICATION_BINARY_DIR}/dts/${platform_dir_name})

	file(MAKE_DIRECTORY ${platform_dtbs_output_dir})

	unset(DTS_ROOT_SYSTEM_INCLUDE_DIRS)
	unset(DTS_ROOT_INCLUDE_DIRS)

	foreach(dts_root ${DTS_ROOT})
		get_filename_component(full_path ${dts_root}/include REALPATH)
		if(EXISTS ${full_path})
			list(APPEND
				DTS_ROOT_SYSTEM_INCLUDE_DIRS
				-isystem ${full_path}
			)
			list(APPEND
				DTS_ROOT_INCLUDE_DIRS
				-i ${full_path}
			)
		endif()
	endforeach()

	get_filename_component(dtsfile ${file} NAME_WE)
	execute_process(
		COMMAND ${CMAKE_DTS_PREPROCESSOR}
		-E   # Stop after preprocessing
		-P
		-Wp,-MD,${platform_dtbs_output_dir}/.${dtsfile}.dtb.d.pre.tmp
		-nostdinc
		${DTS_ROOT_SYSTEM_INCLUDE_DIRS}
		-undef
		-D__DTS__
		-x assembler-with-cpp
		-o ${platform_dtbs_output_dir}/.${dtsfile}.dtb.dts.tmp
		${file}
		WORKING_DIRECTORY ${APPLICATION_SOURCE_DIR}
		RESULT_VARIABLE ret
	)
	if(NOT "${ret}" STREQUAL "0")
		message(WARNING "DTC prepare failed with return code: ${ret}")
		return()
	endif()

	if(DTC)
		execute_process(
			COMMAND ${DTC}
			-O dtb
			-o ${platform_dtbs_output_dir}/${dtsfile}.dtb
			-b 0
			${DTS_ROOT_INCLUDE_DIRS}
			-Wno-unit_address_vs_reg
			-Wno-unit_address_format
			-Wno-avoid_unnecessary_addr_size
			-Wno-alias_paths
			-Wno-graph_child_address
			-Wno-simple_bus_reg
			-Wno-unique_unit_address
			-Wno-pci_device_reg
			-d ${platform_dtbs_output_dir}/.${dtsfile}.dtb.d.dtc.tmp
			${platform_dtbs_output_dir}/.${dtsfile}.dtb.dts.tmp
			OUTPUT_QUIET # Discard stdout
			WORKING_DIRECTORY ${APPLICATION_BINARY_DIR}
			RESULT_VARIABLE ret
		)
		if(NOT "${ret}" STREQUAL "0")
			message(WARNING "DTC build failed with return code: ${ret}")
			return()
		endif()

		execute_process(
			COMMAND cat
			${platform_dtbs_output_dir}/.${dtsfile}.dtb.d.pre.tmp
			${platform_dtbs_output_dir}/.${dtsfile}.dtb.d.dtc.tmp
			OUTPUT_FILE
			${platform_dtbs_output_dir}/.${dtsfile}.dtb.d
			OUTPUT_QUIET # Discard stdout
			WORKING_DIRECTORY ${APPLICATION_BINARY_DIR}
			RESULT_VARIABLE ret
		)
		if(NOT "${ret}" STREQUAL "0")
			message(FATAL_ERROR "DTC finish failed with return code: ${ret}")
		endif()

		toolchain_parse_make_rule(
			${platform_dtbs_output_dir}/.${dtsfile}.dtb.d
			include_files # Output parameter
			)
		set_property(DIRECTORY APPEND PROPERTY
			CMAKE_CONFIGURE_DEPENDS
			${include_files}
			)
		message(STATUS "Generate dtb file: ${platform_dir_name}/${dtsfile}.dtb")
	endif(DTC)
endfunction()

if(NOT DEFINED CMAKE_DTS_PREPROCESSOR)
	set(CMAKE_DTS_PREPROCESSOR ${CMAKE_C_COMPILER})
endif()

rtochius_file(APPLICATION_ROOT DTS_ROOT)
list(APPEND
	DTS_ROOT
	  ${APPLICATION_SOURCE_DIR}/misc/dts
)
list(REMOVE_DUPLICATES DTS_ROOT)

set(platform_dts_files_root_dir ${APPLICATION_SOURCE_DIR}/misc/dts/arch/${ARCH})
add_subdirectory(${platform_dts_files_root_dir})
