#ifndef __RTOCHIUS_SCHED_H_
#define __RTOCHIUS_SCHED_H_

#include <asm/thread_info.h>
#include <asm/processor.h>

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

	/* -1 unrunnable, 0 runnable, >0 stopped: */
	volatile long			state;

	void				*stack;
	/* Per task flags (PF_*), defined further below: */
	unsigned int			flags;

	/* CPU-specific state of this task: */
	struct thread_struct		thread;
};

#endif /* !__RTOCHIUS_SCHED_H_ */
