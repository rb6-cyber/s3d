# Try to find Sqlite3
#  SQLITE3_FOUND - If false, do not try to use SQLITE3.
#  SQLITE3_INCLUDE_DIRS - where to find sqlite3.h
#  SQLITE3_LIBRARIES - the libraries to link against
#  SQLITE3_DEFINITIONS - switches required for SQLite3


if (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIRS)
	# path set by user or was found in the past
	set(SQLITE3_FOUND TRUE)
else (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIRS)
	find_package(PkgConfig)

	pkg_search_module(SQLITE3 sqlite3)
endif (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIRS)
