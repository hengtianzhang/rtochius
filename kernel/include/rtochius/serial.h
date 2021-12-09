/*
 * include/linux/serial.h
 *
 * Copyright (C) 1992 by Theodore Ts'o.
 * 
 * Redistribution of this file is permitted under the terms of the GNU 
 * Public License (GPL)
 */
#ifndef __RTOCHIUS_SERIAL_H_
#define __RTOCHIUS_SERIAL_H_

#include <base/types.h>

#include <rtochius/of.h>

/*
 * Console helpers.
 */
struct earlycon_device {
	void	(*write)(struct earlycon_device *, const char *, unsigned int);
	unsigned long		iobase;			/* in/out[bwl] */
	unsigned char __iomem	*membase;		/* read/write[bwl] */
	char options[16];		/* e.g., 115200n8 */
	unsigned int baud;
	unsigned int clk;
	void	*private_data;
};

struct earlycon_id {
	char	name[15];
	char	name_term;	/* In case compiler didn't '\0' term name */
	char	compatible[128];
	int	(*setup)(struct earlycon_device *, const char *options);
};

extern const struct earlycon_id *__earlycon_table[];
extern const struct earlycon_id *__earlycon_table_end[];

#define EARLYCON_USED_OR_UNUSED	__used

#define _OF_EARLYCON_DECLARE(_name, compat, fn, unique_id)		\
	static const struct earlycon_id unique_id			\
	     EARLYCON_USED_OR_UNUSED __initconst			\
		= { .name = __stringify(_name),				\
		    .compatible = compat,				\
		    .setup = fn  };					\
	static const struct earlycon_id EARLYCON_USED_OR_UNUSED		\
		__section(__earlycon_table)				\
		* const __PASTE(__p, unique_id) = &unique_id

#define OF_EARLYCON_DECLARE(_name, compat, fn)				\
	_OF_EARLYCON_DECLARE(_name, compat, fn,				\
			     __UNIQUE_ID(__earlycon_##_name))

#endif /* !__RTOCHIUS_SERIAL_H_ */
