#include <base/linkage.h>
#include <base/compiler.h>
#include <base/common.h>
#include <base/errno.h>

asmlinkage __visible int printf(const char *fmt, ...)
{
	return 0;
}
