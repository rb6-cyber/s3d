include(CheckCCompilerFlag)

# Enable c99/c9x
check_c_compiler_flag("-std=c9x" SUPPORT_C9X)
if (SUPPORT_C9X)
	list(APPEND CMAKE_C_FLAGS "-std=c9x ")
else (SUPPORT_C9X)
	message(STATUS "Could not enable c9x support")
endif (SUPPORT_C9X)
