# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

project(s3d C CXX)
set(VERSION 0.2.2.1)

cmake_minimum_required(VERSION 3.9)

# set cmake policies
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0072 NEW)


# Add directory with library checks to search path
list(APPEND CMAKE_MODULE_PATH "${s3d_SOURCE_DIR}/cmake/modules")

include(S3DInternals)

# set path to objects
set(OBJECTPATH "${S3D_DATA_INSTALL_DIR}")
STRING(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
if (NOT BUILD_TYPE MATCHES "REL(WITHDEBINFO|EASE)")
	set(OBJECTPATH "${OBJECTPATH}:${s3d_SOURCE_DIR}/objs")
endif (NOT BUILD_TYPE MATCHES "REL(WITHDEBINFO|EASE)")

# set debug level with -DDEBUG=level
include(SetDebugLevel)

# Enable profiling support with -DPROFILING
include(EnableProfiling)

# Set some compiler flags
include(SetCCWarnFlags)

# Enable C9X if possible
# disabled because s3d uses c9x features but doesnt compile with it
#include(SetC9X)

# Run configure stuff
include(ConfigureChecks.cmake)

add_subdirectory(libs3d)
add_subdirectory(libs3dw)
add_subdirectory(apps)
add_subdirectory(cmake)
add_subdirectory(Documentation)
add_subdirectory(example)
add_subdirectory(objs)
add_subdirectory(server)

file(GLOB INCLUDE_EXPERIMENTAL experimental)
if (INCLUDE_EXPERIMENTAL)
	add_subdirectory(experimental)
endif (INCLUDE_EXPERIMENTAL)

# Generate s3drc
configure_file(s3drc.cmake "${s3d_BINARY_DIR}/s3drc")
install(PROGRAMS "${s3d_BINARY_DIR}/s3drc" DESTINATION "${CFG_INSTALL_DIR}")
