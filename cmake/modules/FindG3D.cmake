# Try to find G3D
#  G3D_FOUND - If false, do not try to use G3D.
#  G3D_INCLUDE_DIR - where to find g3d/g3d.h
#  G3D_LIBRARIES - the libraries to link against
#  G3D_DEFINITIONS - switches required for G3D


if (G3D_LIBRARIES AND G3D_INCLUDE_DIR)
	# path set by user or was found in the past
	set(G3D_FOUND TRUE)
else (G3D_LIBRARIES AND G3D_INCLUDE_DIR)
	include(UsePkgConfig)

	pkgconfig(libg3d _IncDir _LinkDir _LinkFlags _CFlags)
	set(G3D_DEFINITIONS ${_CFlags})

	find_path(G3D_INCLUDE_DIR
		NAMES g3d/g3d.h
		PATHS
			${_IncDir}
	)

	find_library(G3D_LIBRARIES
		NAMES g3d
		PATHS ${_LinkDir}
	)

	if (G3D_INCLUDE_DIR AND G3D_LIBRARIES)
		set(G3D_FOUND TRUE)
	endif (G3D_INCLUDE_DIR AND G3D_LIBRARIES)

	if (G3D_FOUND)
		if (NOT G3D_FIND_QUIETLY)
			message(STATUS "Found G3D: ${G3D_LIBRARIES}")
		endif (NOT G3D_FIND_QUIETLY)
	else (G3D_FOUND)
		if (G3D_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find G3D")
		endif (G3D_FIND_REQUIRED)
	endif (G3D_FOUND)

	# set visibility in cache
	set(G3D_DEFINITIONS ${G3D_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(G3D_INCLUDE_DIR G3D_LIBRARIES G3D_DEFINITIONS)

endif (G3D_LIBRARIES AND G3D_INCLUDE_DIR)
