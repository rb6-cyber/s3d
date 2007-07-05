if (PTHREADS_FOUND)
	include_directories(${CMAKE_SOURCE_DIR}/libs3d ${CMAKE_SOURCE_DIR}/libs3dw)

	add_executable(kism3d client.c gui.c kism3d.c network.c)
	target_link_libraries(kism3d s3d s3dw ${PTHREADS_LIBRARIES})

	# install 
	install(TARGETS kism3d RUNTIME DESTINATION bin)
endif (PTHREADS_FOUND)
