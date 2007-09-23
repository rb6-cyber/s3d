project(s3d_x11gate)

if (XTST_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

	add_executable(s3d_x11gate s3d_x11gate.c)
	target_link_libraries(s3d_x11gate s3d s3dw ${XTST_LIBRARIES})

	# install
	install(TARGETS s3d_x11gate RUNTIME DESTINATION bin)
else (XTST_FOUND)
	PkgStatus_Later("xtst (pkg name: libxtst-dev)")
	
	# print late status
	Collected_PkgStatus("s3d_x11gate")
endif (XTST_FOUND)
