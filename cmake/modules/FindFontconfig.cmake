# Try to find Fontconfig
#  FONTCONFIG_FOUND - If false, do not try to use FONTCONFIG.
#  FONTCONFIG_INCLUDE_DIRS - where to find fontconfig/fontconfig.h
#  FONTCONFIG_LIBRARIES - the libraries to link against
#  FONTCONFIG_DEFINITIONS - switches required for FONTCONFIG


if (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIRS)
	# path set by user or was found in the past
	set(FONTCONFIG_FOUND TRUE)
else (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIRS)
	find_package(PkgConfig)

	pkg_search_module(FONTCONFIG fontconfig)
endif (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIRS)
