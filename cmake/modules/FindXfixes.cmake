# Try to find Xfixes
#  XFIXES_FOUND - If false, do not try to use Xfixes
#  XFIXES_LIBRARIES - the libraries to link against
#  XFIXES_DEFINITIONS - switches required for Xfixes


if (XFIXES_LIBRARIES)
	# path set by user or was found in the past
	set(XFIXES_FOUND TRUE)
else (XFIXES_LIBRARIES)
	include(UsePkgConfig)

	pkgconfig(xfixes _IncDir _LinkDir _LinkFlags _CFlags)
	set(XFIXES_DEFINITIONS ${_CFlags})

	find_library(XFIXES_LIBRARIES
		NAMES Xfixes
		PATHS ${_LinkDir}
	)

	if (XFIXES_LIBRARIES)
		set(XFIXES_FOUND TRUE)
	endif (XFIXES_LIBRARIES)

	if (XFIXES_FOUND)
		if (NOT XFIXES_FIND_QUIETLY)
			message(STATUS "Found Xfixes: ${XFIXES_LIBRARIES}")
		endif (NOT XFIXES_FIND_QUIETLY)
	else (XFIXES_FOUND)
		if (XFIXES_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find Xfixes")
		endif (XFIXES_FIND_REQUIRED)
	endif (XFIXES_FOUND)

	# set visibility in cache
	set(XFIXES_DEFINITIONS ${XFIXES_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(XFIXES_LIBRARIES XFIXES_DEFINITIONS)

endif (XFIXES_LIBRARIES)
