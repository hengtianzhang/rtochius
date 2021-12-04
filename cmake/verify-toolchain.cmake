# The purpose of this file is to verify that required variables has been
# defined for proper toolchain use.

# Set internal variables if set in environment.
if(NOT DEFINED RTOCHIUS_TOOLCHAIN)
	set(RTOCHIUS_TOOLCHAIN $ENV{RTOCHIUS_TOOLCHAIN})
endif()

if(NOT RTOCHIUS_TOOLCHAIN AND
   (CROSS_COMPILE OR (DEFINED ENV{CROSS_COMPILE})))
    set(RTOCHIUS_TOOLCHAIN cross-compile)
endif()

if(NOT DEFINED RTOCHIUS_TOOLCHAIN)
	message(FATAL_ERROR "No toolchain defining the available tools, The sample:
    RTOCHIUS_TOOLCHAIN=llvm or CROSS_COMPILE
")
endif()
