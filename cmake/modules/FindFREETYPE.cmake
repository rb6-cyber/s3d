# # Try to find FreeType
#  FREETYPE_FOUND - If false, do not try to use FREETYPE.
#  FREETYPE_INCLUDE_DIRS - where to find freetype/config/ftheader.h and ft2build.h, etc.
#  FREETYPE_LIBRARIES - the libraries to link against
#  FREETYPE_DEFINITIONS - switches required for FREETYPE


if (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIRS)
	# path set by user or was found in the past
	set(FREETYPE_FOUND TRUE)
else (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIRS)
	find_package(PkgConfig)

	pkg_search_module(FREETYPE freetype2)
endif (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIRS)
