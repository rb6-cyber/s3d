# add option for enabling/disabling profiling
option(EXAMPLES "Enable/disable compilation of examples" ON)

if (EXAMPLES)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

	s3d_add_executable(filebrowser filebrowser.c)
	target_link_libraries(filebrowser ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(hudtest hudtest.c)
	target_link_libraries(hudtest ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(katze katze.c)
	target_link_libraries(katze ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(linetest linetest.c)
	target_link_libraries(linetest ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(modelloader modelloader.c)
	target_link_libraries(modelloader ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(nichtsnutz nichtsnutz.c)
	target_link_libraries(nichtsnutz ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(ptrtest ptrtest.c)
	target_link_libraries(ptrtest ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(radius_test radius_test.c)
	target_link_libraries(radius_test ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(s3dclock s3dclock.c)
	target_link_libraries(s3dclock ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(snowman snowman.c)
	target_link_libraries(snowman ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(strtest strtest.c)
	target_link_libraries(strtest ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(texturetest texturetest.c)
	target_link_libraries(texturetest ${S3D_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(widgets widgets.c)
	target_link_libraries(widgets ${S3D_LIBRARIES} ${S3DW_LIBRARIES} ${MATH_LIBRARIES})

	s3d_add_executable(wiresphere wiresphere.c)
	target_link_libraries(wiresphere ${S3D_LIBRARIES} ${MATH_LIBRARIES})
endif (EXAMPLES)
