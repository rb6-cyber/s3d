# Try to find Xtst
#  XTST_FOUND - If false, do not try to use Xtst.
#  XTST_LIBRARIES - the libraries to link against
#  XTST_DEFINITIONS - switches required for Xtst


if (XTST_LIBRARIES)
	# path set by user or was found in the past
	set(XTST_FOUND TRUE)
else (XTST_LIBRARIES)
	set(XTST_DEFINITIONS "")

	find_library(XTST_LIBRARIES
		NAMES Xtst
		PATHS
	)

	if (XTST_LIBRARIES)
		set(XTST_FOUND TRUE)
	endif (XTST_LIBRARIES)

	if (XTST_FOUND)
		if (NOT XTST_FIND_QUIETLY)
			message(STATUS "Found Xtst: ${XTST_LIBRARIES}")
		endif (NOT XTST_FIND_QUIETLY)
	else (XTST_FOUND)
		if (XTST_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find Xtst")
		endif (XTST_FIND_REQUIRED)
	endif (XTST_FOUND)

	# set visibility in cache
	set(XTST_DEFINITIONS ${XTST_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(XTST_LIBRARIES XTST_DEFINITIONS)

endif (XTST_LIBRARIES)
