# add option for enabling/disabling profiling
option(EXAMPLES "Enable/disable compilation of examples" ON)

if (EXAMPLES)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

	add_executable(filebrowser filebrowser.c)
	target_link_libraries(filebrowser s3d ${MATH_LIBRARIES})

	add_executable(hudtest hudtest.c)
	target_link_libraries(hudtest s3d ${MATH_LIBRARIES})

	add_executable(katze katze.c)
	target_link_libraries(katze s3d ${MATH_LIBRARIES})

	add_executable(linetest linetest.c)
	target_link_libraries(linetest s3d ${MATH_LIBRARIES})

	add_executable(modelloader modelloader.c)
	target_link_libraries(modelloader s3d ${MATH_LIBRARIES})

	add_executable(nichtsnutz nichtsnutz.c)
	target_link_libraries(nichtsnutz s3d ${MATH_LIBRARIES})

	add_executable(ptrtest ptrtest.c)
	target_link_libraries(ptrtest s3d ${MATH_LIBRARIES})

	add_executable(radius_test radius_test.c)
	target_link_libraries(radius_test s3d ${MATH_LIBRARIES})

	add_executable(s3dclock s3dclock.c)
	target_link_libraries(s3dclock s3d ${MATH_LIBRARIES})

	add_executable(snowman snowman.c)
	target_link_libraries(snowman s3d ${MATH_LIBRARIES})

	add_executable(strtest strtest.c)
	target_link_libraries(strtest s3d ${MATH_LIBRARIES})

	add_executable(texturetest texturetest.c)
	target_link_libraries(texturetest s3d ${MATH_LIBRARIES})

	add_executable(widgets widgets.c)
	target_link_libraries(widgets s3d s3dw ${MATH_LIBRARIES})

	add_executable(wiresphere wiresphere.c)
	target_link_libraries(wiresphere s3d ${MATH_LIBRARIES})
endif (EXAMPLES)
