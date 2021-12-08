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

#define SERIAL_DECLARE(name, compat, fn) OF_DECLARE_1(serial, name, compat, fn)

#endif /* !__RTOCHIUS_SERIAL_H_ */
