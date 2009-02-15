# Try to find G3D
#  G3D_FOUND - If false, do not try to use G3D.
#  G3D_INCLUDE_DIRS - where to find g3d/g3d.h
#  G3D_LIBRARIES - the libraries to link against
#  G3D_DEFINITIONS - switches required for G3D


if (G3D_LIBRARIES AND G3D_INCLUDE_DIRS)
	# path set by user or was found in the past
	set(G3D_FOUND TRUE)
else (G3D_LIBRARIES AND G3D_INCLUDE_DIRS)
	find_package(PkgConfig)

	pkg_search_module(G3D libg3d>=0.0.7)
endif (G3D_LIBRARIES AND G3D_INCLUDE_DIRS)
