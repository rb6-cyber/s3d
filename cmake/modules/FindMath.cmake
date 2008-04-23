# Try to find libm
#  MATH_FOUND - If false, do not try to use libm
#  MATH_LIBRARIES - the libraries to link against


if (MATH_LIBRARIES)
	# path set by user or was found in the past
	set(MATH_FOUND TRUE)
else (MATH_LIBRARIES)
	find_library(MATH_LIBRARIES
		NAMES m
	)

	if (MATH_LIBRARIES)
		set(MATH_FOUND TRUE)
	endif (MATH_LIBRARIES)

	if (MATH_FOUND)
		if (NOT MATH_FIND_QUIETLY)
			message(STATUS "Found m: ${MATH_LIBRARIES}")
		endif (NOT MATH_FIND_QUIETLY)
	else (MATH_FOUND)
		if (MATH_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find libm")
		endif (MATH_FIND_REQUIRED)
	endif (MATH_FOUND)

	# set visibility in cache
	mark_as_advanced(MATH_LIBRARIES)

endif (MATH_LIBRARIES)
