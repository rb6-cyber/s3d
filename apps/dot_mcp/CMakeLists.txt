project(dot_mcp)

include_directories(${s3d_SOURCE_DIR}/libs3d)

add_executable(dot_mpc main.c menu.c)
target_link_libraries(dot_mpc s3d)

# install 
install(TARGETS dot_mpc RUNTIME DESTINATION bin)
