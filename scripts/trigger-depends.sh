#!/bin/sh
# SPDX-License-Identifier: Apache-2.0

# Error out on error
set -e

rm -f $1/.tmp_trigger_depends
echo 1 > $1/.tmp_trigger_depends
