# Try to find Xcomposite
#  XCOMPOSITE_FOUND - If false, do not try to use Xcomposite
#  XCOMPOSITE_LIBRARIES - the libraries to link against
#  XCOMPOSITE_DEFINITIONS - switches required for Xcomposite


if (XCOMPOSITE_LIBRARIES)
	# path set by user or was found in the past
	set(XCOMPOSITE_FOUND TRUE)
else (XCOMPOSITE_LIBRARIES)
	include(UsePkgConfig)

	pkgconfig(xcomposite _IncDir _LinkDir _LinkFlags _CFlags)
	set(XCOMPOSITE_DEFINITIONS ${_CFlags})

	find_library(XCOMPOSITE_LIBRARIES
		NAMES Xcomposite
		PATHS ${_LinkDir}
	)

	if (XCOMPOSITE_LIBRARIES)
		set(XCOMPOSITE_FOUND TRUE)
	endif (XCOMPOSITE_LIBRARIES)

	if (XCOMPOSITE_FOUND)
		if (NOT XCOMPOSITE_FIND_QUIETLY)
			message(STATUS "Found Xcomposite: ${XCOMPOSITE_LIBRARIES}")
		endif (NOT XCOMPOSITE_FIND_QUIETLY)
	else (XCOMPOSITE_FOUND)
		if (XCOMPOSITE_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find Xcomposite")
		endif (XCOMPOSITE_FIND_REQUIRED)
	endif (XCOMPOSITE_FOUND)

	# set visibility in cache
	set(XCOMPOSITE_DEFINITIONS ${XCOMPOSITE_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(XCOMPOSITE_LIBRARIES XCOMPOSITE_DEFINITIONS)

endif (XCOMPOSITE_LIBRARIES)
