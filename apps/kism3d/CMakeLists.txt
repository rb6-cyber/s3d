# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

project(kism3d C)

if (PTHREADS_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

	add_executable(kism3d client.c gui.c kism3d.c network.c)
	target_link_libraries(kism3d s3d s3dw ${PTHREADS_LIBRARIES}
	                      ${MATH_LIBRARIES})

	# install
	install(TARGETS kism3d RUNTIME DESTINATION "${BIN_INSTALL_DIR}")
else (PTHREADS_FOUND)
	PkgStatus_Later("pthreads")

	# print late status
	Collected_PkgStatus("kism3d")
endif (PTHREADS_FOUND)
