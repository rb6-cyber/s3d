include_directories(${s3d_SOURCE_DIR}/libs3d)

# base files
set(server_src allocate.c cull.c endian.c error.c event.c graphics.c
	main.c matrix.c mcp.c navigation.c network.c object.c
	process.c proto.c shm.c	shm_ringbuf.c tcp.c user.c)

# files for sdl support
set(server_sdl_src graphics_sdl.c user_sdl.c)

# add sdl files and libs
if (SDL_FOUND)
	list(APPEND server_src ${server_sdl_src})
	list(APPEND server_libs ${SDL_LIBRARY})
endif (SDL_FOUND)

# compile base files + sdl specific parts
add_executable(server ${server_src})

target_link_libraries(server ${OPENGL_gl_LIBRARY} ${MATH_LIBRARIES} ${server_libs})
set_target_properties(server PROPERTIES OUTPUT_NAME "s3d")

# install
install(TARGETS server RUNTIME DESTINATION bin)
