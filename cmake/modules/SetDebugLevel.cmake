# if user set debug level
string(TOUPPER "${DEBUG}" DEBUG_UPPER)
set(DEBUG ${DEBUG}
	CACHE STRING "Set Debug Level"
)

# test debug and convert it so numeric debug_level
set(DEBUG_LEVEL CACHE INTERNAL "Numeric representation of DEBUG")
if (DEBUG_UPPER STREQUAL VLOW)
	set(DEBUG_LEVEL "1")
elseif (DEBUG_UPPER STREQUAL LOW)
	set(DEBUG_LEVEL "2")
elseif (DEBUG_UPPER STREQUAL MED)
	set(DEBUG_LEVEL "3")
elseif (DEBUG_UPPER STREQUAL HIGH)
	set(DEBUG_LEVEL "4")
elseif (DEBUG_UPPER STREQUAL VHIGH)
	set(DEBUG_LEVEL "5")
elseif (DEBUG_UPPER STREQUAL "")
	set(DEBUG)
else (DEBUG_UPPER STREQUAL VLOW)
	message(FATAL_ERROR "No valid debug-level [VLOW|LOW|MED|HIGH|VHIGH] found.")
endif (DEBUG_UPPER STREQUAL VLOW)

if (DEBUG)
	message(STATUS "Set debug level to: ${DEBUG}")
endif (DEBUG)