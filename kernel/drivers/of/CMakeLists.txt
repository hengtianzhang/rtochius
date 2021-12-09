

kernel_library()

kernel_library_sources(
	base.c
)

kernel_library_sources_ifdef(CONFIG_OF_FLATTREE
	fdt.c
)
