# SPDX-License-Identifier: Apache-2.0

list(APPEND crt_files arch/${ARCH}/crt0.S)
list(APPEND crt_files arch/${ARCH}/start.c)

add_library(rtochius_runtime_crt_obj OBJECT ${crt_files})

set_property(GLOBAL PROPERTY USERVICE_LINKER_SCRIPT_S "${CMAKE_CURRENT_LIST_DIR}/arch/${ARCH}/linker.lds.S")

add_subdirectory(arch/${ARCH})

target_link_libraries(rtochius_runtime_crt_obj PUBLIC uservice_interface)
target_include_directories(rtochius_runtime_crt_obj
	PUBLIC
	arch/${ARCH}/include
)

find_package(base REQUIRED)
base_user_import_libraries()
target_link_libraries(rtochius_runtime_crt_obj PRIVATE base)

find_package(librtochius REQUIRED)
librtochius_user_import_libraries()
target_link_libraries(rtochius_runtime_crt_obj PRIVATE librtochius)
