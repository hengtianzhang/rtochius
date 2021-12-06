#ifndef __RTOCHIUS_SCHED_H_
#define __RTOCHIUS_SCHED_H_

#include <asm/thread_info.h>

/* Attach to any functions which should be ignored in wchan output. */
#define __sched		__attribute__((__section__(".sched.text")))

/* Linker adds these: start and end of __sched functions */
extern char __sched_text_start[], __sched_text_end[];

struct task_struct {
	/*
	 * For reasons of header soup (see current_thread_info()), this
	 * must be the first element of task_struct.
	 */
	struct thread_info		thread_info;
};

#endif /* !__RTOCHIUS_SCHED_H_ */
