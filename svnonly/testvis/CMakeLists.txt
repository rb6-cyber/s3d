project(testvis C)

if (PTHREADS_FOUND)
	add_executable(testvis testvis.c)
	target_link_libraries(testvis ${PTHREADS_LIBRARIES})

	# install 
	install(TARGETS testvis RUNTIME DESTINATION bin)
else (PTHREADS_FOUND)
	PkgStatus_Later("pthreads")
	
	# print late status
	Collected_PkgStatus("testvis")
endif (PTHREADS_FOUND)
