if (XTST_FOUND)
	include_directories(${CMAKE_SOURCE_DIR}/libs3d ${CMAKE_SOURCE_DIR}/libs3dw)

	add_executable(s3d_x11gate s3d_x11gate.c)
	target_link_libraries(s3d_x11gate s3d s3dw ${XTST_LIBRARIES})

	# install
	install(TARGETS s3d_x11gate RUNTIME DESTINATION bin)
endif (XTST_FOUND)
