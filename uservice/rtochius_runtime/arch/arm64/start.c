#include <base/bug.h>

#include <bootinfo.h>
#include <string.h>

void __rtochius_start_c(void)
{
	int size = __bss_end__ - __bss_start__;

	if (size)
		memset(__bss_start__, 0, size);
}

void __rtochius_exit_c(unsigned long code)
{
	for (;;);
}
