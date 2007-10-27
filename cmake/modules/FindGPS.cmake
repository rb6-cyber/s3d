# Try to find GPS
#  GPS_FOUND - If false, do not try to use GPS.
#  GPS_INCLUDE_DIR - where to find gps.h
#  GPS_LIBRARIES - the libraries to link against


if (GPS_LIBRARIES AND GPS_INCLUDE_DIR)
	# path set by user or was found in the past
	set(GPS_FOUND TRUE)
else (GPS_LIBRARIES AND GPS_INCLUDE_DIR)
	find_path(GPS_INCLUDE_DIR
		NAMES gps.h
	)

	find_library(GPS_LIBRARIES
		NAMES gps
	)

	if (GPS_INCLUDE_DIR AND GPS_LIBRARIES)
		set(GPS_FOUND TRUE)
	endif (GPS_INCLUDE_DIR AND GPS_LIBRARIES)

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
	mark_as_advanced(GPS_INCLUDE_DIR GPS_LIBRARIES)

endif (GPS_LIBRARIES AND GPS_INCLUDE_DIR)
