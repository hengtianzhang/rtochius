# SPDX-License-Identifier: Apache-2.0

# This file provides rtochius Config Package functionality.
#
# The purpose of this files is to allow users to decide if they want to:
# - Use RTOCHIUS_BASE environment setting for explicitly set select a rtochius installation
# - Support automatic rtochius installation lookup through the use of find_package(rtochius)

# First check to see if user has provided a rtochius base manually.
# Set rtochius base to environment setting.
# It will be empty if not set in environment.

macro(include_boilerplate location)
	set(rtochius_FOUND True)
	set(BOILERPLATE_FILE ${RTOCHIUS_BASE}/cmake/app/boilerplate.cmake)

	message("Including boilerplate (${location}): ${BOILERPLATE_FILE}")
	include(${BOILERPLATE_FILE} NO_POLICY_SCOPE)
endmacro()

set(ENV_RTOCHIUS_BASE ${CMAKE_CURRENT_SOURCE_DIR})
if ((NOT DEFINED RTOCHIUS_BASE) AND (DEFINED ENV_RTOCHIUS_BASE))
	# Get rid of any double folder string before comparison, as example, user provides
	# RTOCHIUS_BASE=//path/to//rtochius_base/
	# must also work.
	get_filename_component(RTOCHIUS_BASE ${ENV_RTOCHIUS_BASE} ABSOLUTE)
	set(RTOCHIUS_BASE ${RTOCHIUS_BASE} CACHE PATH "rtochius base")
	include_boilerplate("rtochius base")
	return()
endif()

if(DEFINED RTOCHIUS_BASE)
	include_boilerplate("rtochius base (cached)")
	return()
endif()

message(FATAL_ERROR "rtochius cmake Can't have gotten here.")
