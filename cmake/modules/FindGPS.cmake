# Try to find libgps
#  GPS_FOUND - If false, do not try to use libgps
#  GPS_LIBRARIES - the libraries to link against
#  GPS_DEFINITIONS - switches required for libgps


pkg_search_module(GPS libgps)

if (GPS_FOUND)
	message(STATUS "Check for new style gpsd struct")
	try_compile(CMAKE_GPSD_NEW_STRUCT  ${CMAKE_BINARY_DIR}
		${CMAKE_SOURCE_DIR}/cmake/modules/gpsd_fix.c
		CMAKE_FLAGS "-DLINK_LIBRARIES:STRING=${GPS_LIBRARIES}" "-DINCLUDE_DIRECTORIES=${GPS_INCLUDE_DIRS}")
	if (CMAKE_GPSD_NEW_STRUCT)
		message(STATUS "Check for new style gpsd struct - found")
		set(GPS_NEW_STRUCT 1 CACHE INTERNAL
		"Does the gpsd support new style structs.")
	else (CMAKE_GPSD_NEW_STRUCT)
		message(STATUS "Check for new style gpsd struct - not found")
		set(GPS_NEW_STRUCT 0 CACHE INTERNAL
		"Does the gpsd support new style structs.")
	endif (CMAKE_GPSD_NEW_STRUCT)
endif (GPS_FOUND)
