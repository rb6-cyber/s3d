project(s3d_x11gate C)

if (XTST_FOUND AND XTST_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

	add_executable(s3d_x11gate s3d_x11gate.c)
	target_link_libraries(s3d_x11gate s3d s3dw ${X11_LIBRARIES}
	                      ${XTST_LIBRARIES} ${MATH_LIBRARIES})

	# install
	install(TARGETS s3d_x11gate RUNTIME DESTINATION bin)
else (XTST_FOUND AND XTST_FOUND)
	if (NOT XTST_FOUND)
		PkgStatus_Later("xtst (pkg name: libxtst-dev)")
	endif (NOT XTST_FOUND)

	if (NOT X11_FOUND)
		PkgStatus_Later("x11 (pkg name: libx11-dev)")
	endif (NOT X11_FOUND)

	# print late status
	Collected_PkgStatus("s3d_x11gate")
endif (XTST_FOUND AND XTST_FOUND)
