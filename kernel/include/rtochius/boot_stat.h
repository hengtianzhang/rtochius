#ifndef __RTOCHIUS_BOOT_STAT_H_
#define __RTOCHIUS_BOOT_STAT_H_

#include <base/linkage.h>

extern char *saved_command_line;

extern asmlinkage void start_kernel(void);
extern void setup_arch(char *);

#endif /* !__RTOCHIUS_BOOT_STAT_H_ */
