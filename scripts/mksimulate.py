#!/usr/bin/env python3
#
# Copyright (c) 2017 Intel Corporation.
#
# SPDX-License-Identifier: Apache-2.0
#

import argparse
import os
import stat
import sys


def gen_file(output_file, name):
    output_file.write("""
#!/bin/sh

qemu-system-aarch64 -cpu cortex-a57 -machine type=virt,gic-version=2    \
        -append "rdinit=/linuxrc console=ttyAMA0 earlycon=115200 rodata=nofull" \
        -kernel %s \
        -device virtio-scsi-device  \
        -smp 8  \
        -m 4G   \
        -nographic  \
""" % (name))

    return 0


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument(
        "-n",
        "--name",
        required=True,
        help="Start Image name")

    parser.add_argument(
        "-o",
        "--output",
        required=True,
        help="Output simulate file")

    parser.add_argument(
        "-s",
        "--system",
        required=True,
        help="Qemu system")

    parser.add_argument(
        "-c",
        "--cpu",
        required=True,
        help="Qemu cpu")

    parser.add_argument(
        "-m",
        "--machine",
        required=True,
        help="Qemu machine")

    parser.add_argument(
        "-p",
        "--option",
        required=True,
        help="Qemu virt options")

    parser.add_argument(
        "-a",
        "--append",
        required=True,
        help="Qemu cmdline append")

    parser.add_argument(
        "-r",
        "--initrd",
        required=True,
        help="Qemu initrd")

    parser.add_argument(
        "-M",
        "--memory",
        required=True,
        help="Qemu memory")

    parser.add_argument(
        "-g",
        "--graphic",
        required=True,
        help="Qemu open graphic")

    parser.add_argument(
        "-D",
        "--debug",
        required=True,
        help="Qemu debug")

    args = parser.parse_args()

    output_file = open(args.output, 'w')
    name = args.name
 
    ret = gen_file(output_file, name)

    sys.exit(ret)
