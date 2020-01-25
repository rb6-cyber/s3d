# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

project(meshs3d C)

include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

add_executable(meshs3d hash.c main.c net.c process.c)
target_link_libraries(meshs3d s3d s3dw ${MATH_LIBRARIES})

# install
install(TARGETS meshs3d RUNTIME DESTINATION "${BIN_INSTALL_DIR}")
