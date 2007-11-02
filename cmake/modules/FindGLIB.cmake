# Try to find GLIB2
#  GLIB2_FOUND - If false, do not try to use GLIB2.
#  GLIB2_INCLUDE_DIR - where to find glib.h and glibconfig.h, etc.
#  GLIB2_LIBRARIES - the libraries to link against
#  GLIB2_DEFINITIONS - switches required for GLIB2


if (GLIB2_LIBRARIES AND GLIB2_INCLUDE_DIR)
	# path set by user or was found in the past
	set(GLIB2_FOUND TRUE)
else (GLIB2_LIBRARIES AND GLIB2_INCLUDE_DIR)
	include(UsePkgConfig)

	pkgconfig(glib-2.0 _IncDir _LinkDir _LinkFlags _CFlags)
	set(GLIB2_DEFINITIONS ${_CFlags})

	find_path(GLIB2_INCLUDE_DIR
		NAMES glib.h
		PATHS
			${_IncDir}
			${_IncDir}/glib-2.0
			${_IncDir}/glib-2.0/include
	)

	find_path(GLIBCONFIG_INCLUDE_DIR
		NAMES glibconfig.h
		PATHS
			${_IncDir}
			${_IncDir}/glib-2.0
			${_IncDir}/glib-2.0/include
			${_LinkDir}/glib-2.0
			${_LinkDir}/glib-2.0/include
	)

	find_library(GLIB2_LIBRARIES
		NAMES glib-2.0
		PATHS ${_LinkDir}
	)

	if (GLIB2_INCLUDE_DIR AND GLIBCONFIG_INCLUDE_DIR)
		list(APPEND GLIB2_INCLUDE_DIR ${GLIBCONFIG_INCLUDE_DIR})
	else (GLIB2_INCLUDE_DIR AND GLIBCONFIG_INCLUDE_DIR)
		set(GLIB2_INCLUDE_DIR)
	endif (GLIB2_INCLUDE_DIR AND GLIBCONFIG_INCLUDE_DIR)

	if (GLIB2_INCLUDE_DIR AND GLIB2_LIBRARIES)
		set(GLIB2_FOUND TRUE)
	endif (GLIB2_INCLUDE_DIR AND GLIB2_LIBRARIES)

	if (GLIB2_FOUND)
		if (NOT GLIB2_FIND_QUIETLY)
			message(STATUS "Found GLIB2: ${GLIB2_LIBRARIES}")
		endif (NOT GLIB2_FIND_QUIETLY)
	else (GLIB2_FOUND)
		if (GLIB2_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find GLIB2")
		endif (GLIB2_FIND_REQUIRED)
	endif (GLIB2_FOUND)

	# set visibility in cache
	set(GLIB2_INCLUDE_DIR ${GLIB2_INCLUDE_DIR} CACHE PATH "Path to a file." FORCE)
	set(GLIB2_LIBRARIES ${GLIB2_LIBRARIES} CACHE FILEPATH "Path to a library." FORCE)
	set(GLIB2_DEFINITIONS ${GLIB2_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(GLIB2_INCLUDE_DIR GLIB2_LIBRARIES GLIB2_DEFINITIONS)

	# mark as internal
	set(GLIBCONFIG_INCLUDE_DIR ${GLIBCONFIG_INCLUDE_DIR} CACHE INTERNAL "Path to a config file." FORCE)

endif (GLIB2_LIBRARIES AND GLIB2_INCLUDE_DIR)
