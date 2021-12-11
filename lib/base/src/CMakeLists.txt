
target_sources(
	${base_VAR}base
	PRIVATE
	cpio.c ctype.c string.c find_bit.c bitmap.c cmdline.c
	div64.c kstrtox.c hexdump.c vsprintf.c string_helpers.c
	crc32.c bsearch.c list_sort.c llist.c random.c rbtree.c
	sort.c time.c timeconv.c memblock.c
)

target_sources_ifdef(CONFIG_GENERIC_HWEIGHT
	${base_VAR}base
	PRIVATE
	hweight.c
)

target_sources_ifdef(CONFIG_BITREVERSE
	${base_VAR}base
	PRIVATE
	bitrev.c
)

target_sources_ifdef(CONFIG_SYMBOLIC_ERRNAME
	${base_VAR}base
	PRIVATE
	errname.c
)

add_subdirectory_ifdef(CONFIG_FDT fdt)
