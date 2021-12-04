# SPDX-License-Identifier: Apache-2.0

unset(CMAKE_C_COMPILER)
unset(CMAKE_C_COMPILER CACHE)

unset(CMAKE_CXX_COMPILER)
unset(CMAKE_CXX_COMPILER CACHE)

unset(CMAKE_LINKER)
unset(CMAKE_LINKER CACHE)

# Default toolchain cmake file
set(TOOLCHAIN_ROOT ${RTOCHIUS_BASE})
rtochius_file(APPLICATION_ROOT TOOLCHAIN_ROOT)

# Don't inherit compiler flags from the environment
foreach(var AFLAGS CFLAGS CXXFLAGS CPPFLAGS LDFLAGS)
	if(DEFINED ENV{${var}})
		message(WARNING "The environment variable '${var}' was set to $ENV{${var}},
but rtochius ignores flags from the environment. Use 'cmake -DEXTRA_${var}=$ENV{${var}}' instead.")
		unset(ENV{${var}})
	endif()
endforeach()

set(TOOLCHAIN_ROOT ${TOOLCHAIN_ROOT} CACHE STRING "ochiusrt toolchain root" FORCE)
assert(TOOLCHAIN_ROOT "rtochius toolchain root path invalid: please set the TOOLCHAIN_ROOT-variable")

# Set cached RTOCHIUS_TOOLCHAIN.
set(RTOCHIUS_TOOLCHAIN ${RTOCHIUS_TOOLCHAIN} CACHE STRING "rtochius toolchain variant")

# Configure the toolchain based on what SDK/toolchain is in use.
include(${TOOLCHAIN_ROOT}/cmake/toolchain/${RTOCHIUS_TOOLCHAIN}/generic.cmake)

set_ifndef(TOOLCHAIN_KCONFIG_DIR ${TOOLCHAIN_ROOT}/cmake/toolchain/${RTOCHIUS_TOOLCHAIN})

# Configure the toolchain based on what toolchain technology is used
# (gcc, host-gcc etc.)
include(${TOOLCHAIN_ROOT}/cmake/compiler/${COMPILER}/generic.cmake OPTIONAL)
include(${TOOLCHAIN_ROOT}/cmake/linker/${LINKER}/generic.cmake OPTIONAL)
include(${TOOLCHAIN_ROOT}/cmake/bintools/${BINTOOLS}/generic.cmake OPTIONAL)
