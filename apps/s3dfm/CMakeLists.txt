include_directories(${CMAKE_SOURCE_DIR}/libs3d ${CMAKE_SOURCE_DIR}/libs3dw)

add_executable(s3dfm animation.c box.c dialog.c event.c fly.c focus.c fs.c
	icon.c main.c node.c parse.c string.c)
target_link_libraries(s3dfm s3d s3dw)

# install 
install(TARGETS s3dfm RUNTIME DESTINATION bin)
