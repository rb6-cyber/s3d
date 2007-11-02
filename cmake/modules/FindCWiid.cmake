# Try to find CWiid
#  CWIID_FOUND - If false, do not try to use CWiid.
#  CWIID_INCLUDE_DIR - where to find cwiid.h
#  CWIID_LIBRARIES - the libraries to link against

find_package(Bluetooth)

if (CWIID_LIBRARIES AND CWIID_INCLUDE_DIR AND BLUETOOTH_FOUND)
	# path set by user or was found in the past
	set(CWIID_FOUND TRUE)
else (CWIID_LIBRARIES AND CWIID_INCLUDE_DIR AND BLUETOOTH_FOUND)
	find_path(CWIID_INCLUDE_DIR
		NAMES cwiid.h
	)

	find_library(CWIID_LIBRARIES
		NAMES cwiid
	)

	if (CWIID_INCLUDE_DIR AND CWIID_LIBRARIES AND BLUETOOTH_FOUND)
		set(CWIID_FOUND TRUE)
	endif (CWIID_INCLUDE_DIR AND CWIID_LIBRARIES AND BLUETOOTH_FOUND)

	if (CWIID_FOUND)
		if (NOT CWIID_FIND_QUIETLY)
			message(STATUS "Found CWiid: ${CWIID_LIBRARIES}")
		endif (NOT CWIID_FIND_QUIETLY)
	else (CWIID_FOUND)
		if (CWIID_FIND_REQUIRED)
			if (NOT BLUETOOTH_FOUND)
				message(FATAL_ERROR "Could not find dependency Bluetooth for CWiid")
			else (NOT BLUETOOTH_FOUND)
				message(FATAL_ERROR "Could not find CWiid")
			endif (NOT BLUETOOTH_FOUND)
		endif (CWIID_FIND_REQUIRED)
	endif (CWIID_FOUND)

	# set visibility in cache
	mark_as_advanced(CWIID_INCLUDE_DIR CWIID_LIBRARIES)

endif (CWIID_LIBRARIES AND CWIID_INCLUDE_DIR AND BLUETOOTH_FOUND)
