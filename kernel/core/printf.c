#include <base/linkage.h>
#include <base/compiler.h>
#include <base/common.h>
#include <base/errno.h>

#include <rtochius/serial.h>
#include <rtochius/spinlock.h>

#define pr_fmt(fmt) "printf" ": " fmt

/*
 * The logbuf_lock protects kmsg buffer, indices, counters.  This can be taken
 * within the scheduler's rq lock. It must be released before calling
 * console_unlock() or anything else that might wake up a process.
 */
DEFINE_RAW_SPINLOCK(logbuf_lock);

enum log_flags {
	LOG_NEWLINE	= 2,	/* text ended with a newline */
	LOG_PREFIX	= 4,	/* text started with a prefix */
	LOG_CONT	= 8,	/* text is a fragment of a continuation line */
};

struct printf_log {
	u64 ts_nsec;		/* timestamp in nanoseconds */
	u16 len;		/* length of entire record */
	u16 text_len;		/* length of text buffer */
	u16 dict_len;		/* length of dictionary buffer */
	u8 facility;		/* syslog facility */
	u8 flags:5;		/* internal record flags */
	u8 level:3;		/* syslog level */
}
#ifdef CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS
__packed __aligned(4)
#endif
;

/* record buffer */
#define LOG_ALIGN __alignof__(struct printf_log)
#define __LOG_BUF_LEN (1 << CONFIG_LOG_BUF_SHIFT)
#define LOG_BUF_LEN_MAX (u32)(1 << 31)
static char __log_buf[__LOG_BUF_LEN] __aligned(LOG_ALIGN);
static char *log_buf = __log_buf;
static u32 log_buf_len = __LOG_BUF_LEN;

static __printf(1, 0) int vprintf_func(const char *fmt, va_list args)
{
	int level = LOGLEVEL_DEFAULT;

	return 0;
}

asmlinkage __visible int printf(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintf_func(fmt, args);
	va_end(args);

	return r;
}
