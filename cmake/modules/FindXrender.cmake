# Try to find Xrender
#  XRENDER_FOUND - If false, do not try to use Xrender
#  XRENDER_LIBRARIES - the libraries to link against
#  XRENDER_DEFINITIONS - switches required for Xrender


if (XRENDER_LIBRARIES)
	# path set by user or was found in the past
	set(XRENDER_FOUND TRUE)
else (XRENDER_LIBRARIES)
	include(UsePkgConfig)

	pkgconfig(xrender _IncDir _LinkDir _LinkFlags _CFlags)
	set(XRENDER_DEFINITIONS ${_CFlags})

	find_library(XRENDER_LIBRARIES
		NAMES Xrender
		PATHS ${_LinkDir}
	)

	if (XRENDER_LIBRARIES)
		set(XRENDER_FOUND TRUE)
	endif (XRENDER_LIBRARIES)

	if (XRENDER_FOUND)
		if (NOT XRENDER_FIND_QUIETLY)
			message(STATUS "Found Xrender: ${XRENDER_LIBRARIES}")
		endif (NOT XRENDER_FIND_QUIETLY)
	else (XRENDER_FOUND)
		if (XRENDER_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find Xrender")
		endif (XRENDER_FIND_REQUIRED)
	endif (XRENDER_FOUND)

	# set visibility in cache
	set(XRENDER_DEFINITIONS ${XRENDER_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(XRENDER_LIBRARIES XRENDER_DEFINITIONS)

endif (XRENDER_LIBRARIES)
