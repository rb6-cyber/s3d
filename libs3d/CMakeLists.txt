include_directories(${s3d_SOURCE_DIR}/server)

set(S3D_LIBRARY_DEPENDENCIES  ${MATH_LIBRARIES} ${G3D_LIBRARIES} ${FONTCONFIG_LIBRARIES} ${FREETYPE_LIBRARIES})

add_library(s3d SHARED callback.c endian.c error.c event.c fontselect.c
	freetype.c io.c modelread.c network.c object_queue.c proto_in.c
	proto_out.c sei_construct.c sei_misc.c sei_monotone.c sei_tri.c shm.c
	shm_ringbuf.c tcp.c vector.c hash.c texture.c)
target_link_libraries(s3d ${S3D_LIBRARY_DEPENDENCIES})
set_target_properties(s3d PROPERTIES VERSION ${VERSION} SOVERSION 2
                      LINK_INTERFACE_LIBRARIES "")

# install
install(TARGETS s3d
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

install(FILES s3d.h s3d_keysym.h DESTINATION include/)

#pkg config
configure_file(libs3d.pc.cmake ${s3d_BINARY_DIR}/libs3d/libs3d.pc @ONLY)
install(FILES ${s3d_BINARY_DIR}/libs3d/libs3d.pc DESTINATION lib/pkgconfig/)
