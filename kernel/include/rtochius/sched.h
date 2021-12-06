#ifndef __RTOCHIUS_SCHED_H_
#define __RTOCHIUS_SCHED_H_

/* Attach to any functions which should be ignored in wchan output. */
#define __sched		__attribute__((__section__(".sched.text")))

/* Linker adds these: start and end of __sched functions */
extern char __sched_text_start[], __sched_text_end[];

#endif /* !__RTOCHIUS_SCHED_H_ */
