#include <base/linkage.h>

ENTRY(_start)
	sub	sp, sp, #8
	bl	__rtochius_start_c
	bl	main
	bl	__rtochius_exit_c
ENDPROC(_start)
