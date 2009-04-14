# Try to find GPS
#  GPS_FOUND - If false, do not try to use GPS.
#  GPS_INCLUDE_DIRS - where to find gps.h
#  GPS_LIBRARIES - the libraries to link against
#  GPS_NEW_STRUCT - if new style gps_t struct was found


if (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)
	# path set by user or was found in the past
	set(GPS_FOUND TRUE)
else (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)
	find_path(GPS_INCLUDE_DIRS
		NAMES gps.h
	)

	find_library(GPS_LIBRARIES
		NAMES gps
	)

	if (GPS_INCLUDE_DIRS AND GPS_LIBRARIES)
		set(GPS_FOUND TRUE)
	endif (GPS_INCLUDE_DIRS AND GPS_LIBRARIES)

	if (GPS_FOUND)
		if (NOT GPS_FIND_QUIETLY)
			message(STATUS "Found GPS: ${GPS_LIBRARIES}")
		endif (NOT GPS_FIND_QUIETLY)
	else (GPS_FOUND)
		if (GPS_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find GPS")
		endif (GPS_FIND_REQUIRED)
	endif (GPS_FOUND)

	# set visibility in cache
	mark_as_advanced(GPS_INCLUDE_DIRS GPS_LIBRARIES)

endif (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)

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
