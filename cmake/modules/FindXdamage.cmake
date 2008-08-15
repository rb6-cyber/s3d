# Try to find Xdamage
#  XDAMAGE_FOUND - If false, do not try to use Xdamage
#  XDAMAGE_LIBRARIES - the libraries to link against
#  XDAMAGE_DEFINITIONS - switches required for Xdamage


if (XDAMAGE_LIBRARIES)
	# path set by user or was found in the past
	set(XDAMAGE_FOUND TRUE)
else (XDAMAGE_LIBRARIES)
	set(XDAMAGE_DEFINITIONS "")

	find_library(XDAMAGE_LIBRARIES
		NAMES Xdamage
		PATHS
	)

	if (XDAMAGE_LIBRARIES)
		set(XDAMAGE_FOUND TRUE)
	endif (XDAMAGE_LIBRARIES)

	if (XDAMAGE_FOUND)
		if (NOT XDAMAGE_FIND_QUIETLY)
			message(STATUS "Found Xdamage: ${XDAMAGE_LIBRARIES}")
		endif (NOT XDAMAGE_FIND_QUIETLY)
	else (XDAMAGE_FOUND)
		if (XDAMAGE_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find Xdamage")
		endif (XDAMAGE_FIND_REQUIRED)
	endif (XDAMAGE_FOUND)

	# set visibility in cache
	set(XDAMAGE_DEFINITIONS ${XDAMAGE_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(XDAMAGE_LIBRARIES XDAMAGE_DEFINITIONS)

endif (XDAMAGE_LIBRARIES)
