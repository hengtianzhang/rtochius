/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_PARAM_H_
#define __RTOCHIUS_PARAM_H_

struct obs_kernel_param {
	const char *str;
	int (*setup_func)(char *);
};

#define __setup_param(str, unique_id, fn)		\
	static const char __setup_str_##unique_id[] __initconst	\
		__aligned(1) = str;						\
	static struct obs_kernel_param __setup_##unique_id	\
		__used __section(.init.setup)			\
		__attribute__((aligned((sizeof(long)))))		\
		= {  __setup_str_##unique_id, fn}

#define early_param(str, fn)					\
	__setup_param(str, fn, fn)

extern const struct obs_kernel_param __setup_start[], __setup_end[];

extern bool parameqn(const char *a, const char *b, size_t n);

extern bool parameq(const char *a, const char *b);

extern char *parse_args(const char *doing,
			char *args,
			void *arg,
			int (*unknown)(char *param, char *val,
					const char *doing, void *arg));

extern void parse_early_options(char *cmdline);
extern void parse_early_param(void);

#endif /* !__RTOCHIUS_PARAM_H_ */
