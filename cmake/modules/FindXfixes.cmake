# Try to find Xfixes
#  XFIXES_FOUND - If false, do not try to use Xfixes
#  XFIXES_LIBRARIES - the libraries to link against
#  XFIXES_DEFINITIONS - switches required for Xfixes


if (XFIXES_LIBRARIES)
	# path set by user or was found in the past
	set(XFIXES_FOUND TRUE)
else (XFIXES_LIBRARIES)
	find_package(PkgConfig)

	pkg_search_module(XFIXES xfixes)
endif (XFIXES_LIBRARIES)
