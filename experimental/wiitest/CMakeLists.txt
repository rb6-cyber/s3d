# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

project(wiitest C)

if (CWIID_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${CWIID_INCLUDE_DIRS})

	add_executable(wiitest wiitest.c)
	target_link_libraries(wiitest s3d ${CWIID_LIBRARIES} ${MATH_LIBRARIES})

	# install
	install(TARGETS wiitest RUNTIME DESTINATION "${BIN_INSTALL_DIR}")
else (CWIID_FOUND)
	if (NOT CWIID_FOUND)
		PkgStatus_Later("libcwiid (pkg name: libcwiid-dev)")
	endif (NOT CWIID_FOUND)
	
	# print late status
	Collected_PkgStatus("wiitest")
endif (CWIID_FOUND)
