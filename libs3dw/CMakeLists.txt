include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

add_library(s3dw SHARED animate.c arrange.c button.c event.c input.c
	label.c root.c scrollbar.c style.c surface.c textbox.c widget.c)

# install
install(TARGETS s3dw
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

install(FILES s3dw.h DESTINATION include/)
