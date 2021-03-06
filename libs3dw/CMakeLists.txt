# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

set(S3DW_LIBRARY_DEPENDENCIES s3d ${MATH_LIBRARIES})

include(FilterPIE)
add_library(s3dw SHARED animate.c arrange.c button.c event.c input.c
	label.c root.c scrollbar.c style.c surface.c textbox.c widget.c)
target_link_libraries(s3dw ${S3DW_LIBRARY_DEPENDENCIES})
set_target_properties(s3dw PROPERTIES VERSION ${VERSION} SOVERSION 2
                      LINK_INTERFACE_LIBRARIES "")

include(${CMAKE_ROOT}/Modules/TestCXXAcceptsFlag.cmake)
check_cxx_accepts_flag("-Wl,--version-script=${s3d_SOURCE_DIR}/libs3dw/libs3dw.ver" HAVE_LIBS3DW_VERSIONSCRIPT)
if (HAVE_LIBS3DW_VERSIONSCRIPT)
set_target_properties(s3dw PROPERTIES LINK_FLAGS "-Wl,--version-script=${s3d_SOURCE_DIR}/libs3dw/libs3dw.ver")
endif (HAVE_LIBS3DW_VERSIONSCRIPT)

# install
install(TARGETS s3dw
	RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
	LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
	ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
)

install(FILES s3dw.h DESTINATION "${INCLUDE_INSTALL_DIR}")

#pkg config
configure_file(libs3dw.pc.cmake ${s3d_BINARY_DIR}/libs3dw/libs3dw.pc @ONLY)
install(FILES ${s3d_BINARY_DIR}/libs3dw/libs3dw.pc DESTINATION "${PKGCFG_INSTALL_DIR}")
