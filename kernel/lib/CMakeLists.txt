
kernel_interface_library()

kernel_interface_library_sources(
    test.c
)

kernel_interface_library_sources_ifdef(
    CONFIG_MEMTEST
    memtest.c
)
