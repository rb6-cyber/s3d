# Try to find Sqlite3
#  SQLITE3_FOUND - If false, do not try to use SQLITE3.
#  SQLITE3_INCLUDE_DIR - where to find sqlite3.h
#  SQLITE3_LIBRARIES - the libraries to link against
#  SQLITE3_DEFINITIONS - switches required for SQLite3


if (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIR)
	# path set by user or was found in the past
	set(SQLITE3_FOUND TRUE)
else (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIR)
	include(UsePkgConfig)

	pkgconfig(sqlite3 _IncDir _LinkDir _LinkFlags _CFlags)
	set(SQLITE3_DEFINITIONS ${_CFlags})

	find_path(SQLITE3_INCLUDE_DIR
		NAMES sqlite3.h
		PATHS
			${_IncDir}
	)

	find_library(SQLITE3_LIBRARIES
		NAMES sqlite3
		PATHS ${_LinkDir}
	)

	if (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)
		set(SQLITE3_FOUND TRUE)
	endif (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)

	if (SQLITE3_FOUND)
		if (NOT SQLITE3_FIND_QUIETLY)
			message(STATUS "Found SQLITE3: ${SQLITE3_LIBRARIES}")
		endif (NOT SQLITE3_FIND_QUIETLY)
	else (SQLITE3_FOUND)
		if (SQLITE3_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find SQLite3")
		endif (SQLITE3_FIND_REQUIRED)
	endif (SQLITE3_FOUND)

	# set visibility in cache
	set(SQLITE3_DEFINITIONS ${SQLITE3_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(SQLITE3_INCLUDE_DIR SQLITE3_LIBRARIES SQLITE3_DEFINITIONS)

endif (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIR)
