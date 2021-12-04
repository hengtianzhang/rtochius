#!/usr/bin/env python3
#
# Copyright (c) 2017 Intel Corporation.
#
# SPDX-License-Identifier: Apache-2.0
#

import argparse
import sys


def gen_file(output_file):
    output_file.write("""
/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __GENERATED_KCONFIG_H_
#define __GENERATED_KCONFIG_H_

#include <generated/autoconf.h>

#ifdef CONFIG_CPU_BIG_ENDIAN
#define __BIG_ENDIAN 4321
#else
#define __LITTLE_ENDIAN 1234
#endif

#define __ARG_PLACEHOLDER_1 0,
#define __take_second_arg(__ignored, val, ...) val

/*
 * The use of "&&" / "||" is limited in certain expressions.
 * The following enable to calculate "and" / "or" with macro expansion only.
 */
#define __and(x, y)			___and(x, y)
#define ___and(x, y)			____and(__ARG_PLACEHOLDER_##x, y)
#define ____and(arg1_or_junk, y)	__take_second_arg(arg1_or_junk y, 0)

#define __or(x, y)			___or(x, y)
#define ___or(x, y)			____or(__ARG_PLACEHOLDER_##x, y)
#define ____or(arg1_or_junk, y)		__take_second_arg(arg1_or_junk 1, y)

/*
 * Helper macros to use CONFIG_ options in C/CPP expressions. Note that
 * these only work with boolean and tristate options.
 */

/*
 * Getting something that works in C and CPP for an arg that may or may
 * not be defined is tricky.  Here, if we have "#define CONFIG_BOOGER 1"
 * we match on the placeholder define, insert the "0," for arg1 and generate
 * the triplet (0, 1, 0).  Then the last step cherry picks the 2nd arg (a one).
 * When CONFIG_BOOGER is not defined, we generate a (... 1, 0) pair, and when
 * the last step cherry picks the 2nd arg, we get a zero.
 */
#define __is_defined(x)			___is_defined(x)
#define ___is_defined(val)		____is_defined(__ARG_PLACEHOLDER_##val)
#define ____is_defined(arg1_or_junk)	__take_second_arg(arg1_or_junk 1, 0)

/*
 * IS_BUILTIN(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y', 0
 * otherwise. For boolean options, this is equivalent to
 * IS_ENABLED(CONFIG_FOO).
 */
#define IS_BUILTIN(option) __is_defined(option)

/*
 * IS_ENABLED(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y',
 * 0 otherwise.
 */
#define IS_ENABLED(option) IS_BUILTIN(option)

#endif /* !__GENERATED_KCONFIG_H_ */
""")

    return 0


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument(
        "-o",
        "--output",
        required=True,
        help="Output header file")

    args = parser.parse_args()

    output_file = open(args.output, 'w')

    ret = gen_file(output_file)

    sys.exit(ret)
