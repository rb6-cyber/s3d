# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

project(dot_mcp C)

include_directories(${s3d_SOURCE_DIR}/libs3d)

add_executable(dot_mcp main.c menu.c)
target_link_libraries(dot_mcp s3d ${MATH_LIBRARIES})

# install
install(TARGETS dot_mcp RUNTIME DESTINATION "${BIN_INSTALL_DIR}")
