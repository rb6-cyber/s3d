include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

add_library(s3dw SHARED animate.c arrange.c button.c event.c input.c
	label.c root.c scrollbar.c style.c surface.c textbox.c widget.c)
set(S3DW_LIBRARIES s3dw CACHE PATHS "Path to libraries")

# install
install(TARGETS s3dw
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

install(FILES s3dw.h DESTINATION include/)

#pkg config
configure_file(libs3dw.pc.cmake ${s3d_BINARY_DIR}/libs3dw/libs3dw.pc @ONLY)
install(FILES ${s3d_BINARY_DIR}/libs3dw/libs3dw.pc DESTINATION lib/pkgconfig/)
