
target_sources(${base_VAR}base
	PRIVATE
	fdt.c fdt_ro.c fdt_wip.c fdt_rw.c
	fdt_sw.c fdt_strerror.c fdt_empty_tree.c
)

target_include_directories(${base_VAR}base
	PRIVATE
	libfdt
)
