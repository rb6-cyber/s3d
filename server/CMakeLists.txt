include_directories(${CMAKE_SOURCE_DIR}/libs3d)

# base files
set(server_src allocate.c cull.c error.c event.c graphics.c
	main.c matrix.c mcp.c navigation.c network.c object.c
	process.c proto.c shm.c	shm_ringbuf.c tcp.c user.c)

# files for sdl support
set(server_sdl_src graphics_sdl.c user_sdl.c)

# files for glut support
set(server_glut_src graphics_glut.c user_glut.c)

# add glut files and libs when glut was found
if (GLUT_FOUND)
	list(APPEND server_src ${server_glut_src})
	list(APPEND server_libs ${GLUT_glut_LIBRARY})
endif (GLUT_FOUND)

# add sdl files and libs when sdl was found
if (SDL_FOUND)
	list(APPEND server_src ${server_sdl_src})
	list(APPEND server_libs ${SDL_LIBRARY})
endif (SDL_FOUND)

# compile base files + sdl or glut specific parts
add_executable(server ${server_src})

target_link_libraries(server ${OPENGL_LIBRARY} ${server_libs})
set_target_properties(server PROPERTIES OUTPUT_NAME "s3d")

# install 
install(TARGETS server RUNTIME DESTINATION bin)
