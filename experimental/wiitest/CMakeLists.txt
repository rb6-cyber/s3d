project(wiitest C)

if (CWIID_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${CWIID_INCLUDE_DIR})

	add_executable(wiitest wiitest.c)
	target_link_libraries(wiitest ${S3D_LIBRARIES} ${CWIID_LIBRARIES} ${MATH_LIBRARIES})

	# install
	install(TARGETS wiitest RUNTIME DESTINATION "${BIN_INSTALL_DIR}")
else (CWIID_FOUND)
	if (NOT CWIID_FOUND)
		if (NOT BLUETOOTH_FOUND)
			PkgStatus_Later("libbluetooth (pkg name: libbluetooth-dev)")
		endif (NOT BLUETOOTH_FOUND)

		PkgStatus_Later("libcwiid (pkg name: libcwiid1-dev)")
	endif (NOT CWIID_FOUND)
	
	# print late status
	Collected_PkgStatus("wiitest")
endif (CWIID_FOUND)
