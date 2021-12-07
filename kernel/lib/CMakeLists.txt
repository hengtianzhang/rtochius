
kernel_interface_library()

kernel_interface_library_sources(
    cpumask.c
)

kernel_interface_library_sources_ifdef(
    CONFIG_MEMTEST
    memtest.c
)

find_package(base REQUIRED)
base_kernel_import_libraries()
