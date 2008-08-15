# Try to find Xcomposite
#  XCOMPOSITE_FOUND - If false, do not try to use Xcomposite
#  XCOMPOSITE_LIBRARIES - the libraries to link against
#  XCOMPOSITE_DEFINITIONS - switches required for Xcomposite


if (XCOMPOSITE_LIBRARIES)
	# path set by user or was found in the past
	set(XCOMPOSITE_FOUND TRUE)
else (XCOMPOSITE_LIBRARIES)
	find_package(PkgConfig)

	pkg_search_module(XCOMPOSITE xcomposite)
endif (XCOMPOSITE_LIBRARIES)
