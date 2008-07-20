option(BUILD_EXPERIMENTAL "Enable/disable build of experimental programs" ON)

if (BUILD_EXPERIMENTAL)
	add_subdirectory(comptest)
	add_subdirectory(olsrs3d)
	add_subdirectory(testvis)
	add_subdirectory(wiitest)
endif (BUILD_EXPERIMENTAL)
