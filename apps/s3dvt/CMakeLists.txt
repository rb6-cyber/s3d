project(s3dvt)

if (PTHREADS_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d)

	add_executable(s3dvt main.c terminal.c)
	target_link_libraries(s3dvt s3d ${PTHREADS_LIBRARIES})

	# install
	install(TARGETS s3dvt RUNTIME DESTINATION bin)
endif (PTHREADS_FOUND)
