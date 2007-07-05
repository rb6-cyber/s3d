# Try to find PThreads
#  PTHREADS_FOUND - If false, do not try to use PThreads.
#  PTHREADS_INCLUDE_DIR - where to find pthreads.h
#  PTHREADS_LIBRARIES - the libraries to link against


if (PTHREADS_LIBRARIES AND PTHREADS_INCLUDE_DIR)
	# path set by user or was found in the past
	set(PTHREADS_FOUND TRUE)
else (PTHREADS_LIBRARIES AND PTHREADS_INCLUDE_DIR)
	find_path(PTHREADS_INCLUDE_DIR
		NAMES pthread.h
	)

	find_library(PTHREADS_LIBRARIES
		NAMES pthread
	)

	if (NOT PTHREADS_LIBRARIES)
		find_library(PTHREADS_LIBRARIES
			NAMES pthreads
		)
	endif (NOT PTHREADS_LIBRARIES)

	if (PTHREADS_INCLUDE_DIR AND PTHREADS_LIBRARIES)
		set(PTHREADS_FOUND TRUE)
	endif (PTHREADS_INCLUDE_DIR AND PTHREADS_LIBRARIES)

	if (PTHREADS_FOUND)
		if (NOT PTHREADS_FIND_QUIETLY)
			message(STATUS "Found PThreads: ${PTHREADS_LIBRARIES}")
		endif (NOT PTHREADS_FIND_QUIETLY)
	else (PTHREADS_FOUND)
		if (PTHREADS_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find PThreads")
		endif (PTHREADS_FIND_REQUIRED)
	endif (PTHREADS_FOUND)

	# set visibility in cache
 	mark_as_advanced(PTHREADS_INCLUDE_DIR PTHREADS_LIBRARIES)

endif (PTHREADS_LIBRARIES AND PTHREADS_INCLUDE_DIR)
