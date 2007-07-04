include_directories(${CMAKE_SOURCE_DIR}/libs3d)

add_executable(s3dvt main.c terminal.c)
target_link_libraries(s3dvt s3d)

# install
install(TARGETS s3dvt RUNTIME DESTINATION bin)
