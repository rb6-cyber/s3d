# Try to find Xrender
#  XRENDER_FOUND - If false, do not try to use Xrender
#  XRENDER_LIBRARIES - the libraries to link against
#  XRENDER_DEFINITIONS - switches required for Xrender


if (XRENDER_LIBRARIES)
	# path set by user or was found in the past
	set(XRENDER_FOUND TRUE)
else (XRENDER_LIBRARIES)
	find_package(PkgConfig)

	pkg_search_module(XRENDER xrender)	
endif (XRENDER_LIBRARIES)
