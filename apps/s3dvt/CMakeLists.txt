project(s3dvt C)

if (PTHREADS_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d)

	add_executable(s3dvt main.c terminal.c)
	target_link_libraries(s3dvt s3d ${PTHREADS_LIBRARIES} ${MATH_LIBRARIES})

	# install
	install(TARGETS s3dvt RUNTIME DESTINATION bin)
else (PTHREADS_FOUND)
	PkgStatus_Later("pthreads")

	# print late status
	Collected_PkgStatus("s3dvt")
endif (PTHREADS_FOUND)
