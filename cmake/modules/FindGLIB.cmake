# Try to find GLIB2
#  GLIB2_FOUND - If false, do not try to use GLIB2.
#  GLIB2_INCLUDE_DIRS - where to find glib.h and glibconfig.h, etc.
#  GLIB2_LIBRARIES - the libraries to link against
#  GLIB2_DEFINITIONS - switches required for GLIB2


if (GLIB2_LIBRARIES AND GLIB2_INCLUDE_DIRS)
	# path set by user or was found in the past
	set(GLIB2_FOUND TRUE)
else (GLIB2_LIBRARIES AND GLIB2_INCLUDE_DIRS)
	find_package(PkgConfig)

	pkg_search_module(GLIB2 glib-2.0)
endif (GLIB2_LIBRARIES AND GLIB2_INCLUDE_DIRS)
