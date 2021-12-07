#include <base/linkage.h>
#include <base/common.h>
#include <base/compiler.h>
#include <base/init.h>

#include <asm/stackprotector.h>

unsigned long long __stack_chk_guard;

#ifdef CONFIG_STACKPROTECTOR
/*
 * Called when gcc's -fstack-protector feature is used, and
 * gcc detects corruption of the on-stack canary value
 */
__visible void __stack_chk_fail(void)
{
	panic("stack-protector: Kernel stack is corrupted in: %pB",
		__builtin_return_address(0));
}
#endif

void panic(const char *fmt, ...)
{
	for (;;);
}
