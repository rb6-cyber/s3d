project(s3d C)
set(VERSION 0.2.1)

# we require cmake 2.4.4 or higher
cmake_minimum_required(VERSION 2.4.4)

# Add directory with library checks to search path
set(CMAKE_MODULE_PATH "${s3d_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

# set path to objects
set(OBJECTPATH "${CMAKE_INSTALL_PREFIX}/share/s3d/objs:${s3d_SOURCE_DIR}/objs")

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

add_subdirectory(apps)
add_subdirectory(svnonly)
add_subdirectory(cmake)
add_subdirectory(Documentation)
add_subdirectory(example)
add_subdirectory(libs3d)
add_subdirectory(libs3dw)
add_subdirectory(objs)
add_subdirectory(server)

# Generate s3drc
configure_file(s3drc.cmake ${s3d_BINARY_DIR}/s3drc)
install(FILES ${s3d_BINARY_DIR}/s3drc DESTINATION /etc)
