include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

set(S3DW_LIBRARY_DEPENDENCIES s3d ${MATH_LIBRARIES})

add_library(s3dw SHARED animate.c arrange.c button.c event.c input.c
	label.c root.c scrollbar.c style.c surface.c textbox.c widget.c)
target_link_libraries(s3dw ${S3DW_LIBRARY_DEPENDENCIES})
set_target_properties(s3dw PROPERTIES VERSION ${VERSION} SOVERSION 2
                      LINK_INTERFACE_LIBRARIES "")

# install
install(TARGETS s3dw
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

install(FILES s3dw.h DESTINATION include/)

#pkg config
configure_file(libs3dw.pc.cmake ${s3d_BINARY_DIR}/libs3dw/libs3dw.pc @ONLY)
install(FILES ${s3d_BINARY_DIR}/libs3dw/libs3dw.pc DESTINATION lib/pkgconfig/)
