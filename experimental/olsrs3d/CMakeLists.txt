# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

project(olsrs3d C)

include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

add_executable(olsrs3d main.c net.c process.c search.c)
target_link_libraries(olsrs3d s3d s3dw ${MATH_LIBRARIES})

# install
install(TARGETS olsrs3d RUNTIME DESTINATION "${BIN_INSTALL_DIR}")
