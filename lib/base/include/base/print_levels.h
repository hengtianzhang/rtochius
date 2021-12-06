/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __BASE_PRINT_LEVELS_H_
#define __BASE_PRINT_LEVELS_H_

#define PRINT_SOH	"\001"		/* ASCII Start Of Header */
#define PRINT_SOH_ASCII	'\001'

#define PRINT_EMERG	PRINT_SOH "0"	/* system is unusable */
#define PRINT_ALERT	PRINT_SOH "1"	/* action must be taken immediately */
#define PRINT_CRIT	PRINT_SOH "2"	/* critical conditions */
#define PRINT_ERR	PRINT_SOH "3"	/* error conditions */
#define PRINT_WARNING	PRINT_SOH "4"	/* warning conditions */
#define PRINT_NOTICE	PRINT_SOH "5"	/* normal but significant condition */
#define PRINT_INFO	PRINT_SOH "6"	/* informational */
#define PRINT_DEBUG	PRINT_SOH "7"	/* debug-level messages */

#define PRINT_DEFAULT	""		/* the default kernel loglevel */

/*
 * Annotation for a "continued" line of log printout (only done after a
 * line that had no enclosing \n). Only to be used by core/arch code
 * during early bootup (a continued line is not SMP-safe otherwise).
 */
#define PRINT_CONT	PRINT_SOH "c"

/* integer equivalents of PRINT_<LEVEL> */
#define LOGLEVEL_SCHED		-2	/* Deferred messages from sched code
					 * are set to this special level */
#define LOGLEVEL_DEFAULT	-1	/* default (or last) loglevel */
#define LOGLEVEL_EMERG		0	/* system is unusable */
#define LOGLEVEL_ALERT		1	/* action must be taken immediately */
#define LOGLEVEL_CRIT		2	/* critical conditions */
#define LOGLEVEL_ERR		3	/* error conditions */
#define LOGLEVEL_WARNING	4	/* warning conditions */
#define LOGLEVEL_NOTICE		5	/* normal but significant condition */
#define LOGLEVEL_INFO		6	/* informational */
#define LOGLEVEL_DEBUG		7	/* debug-level messages */

#endif /* !__BASE_PRINT_LEVELS_H_ */
