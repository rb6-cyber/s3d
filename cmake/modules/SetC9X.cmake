# Enable c99/c9x
check_c_compiler_flag("-std=c9x" SUPPORT_C9X)
if (SUPPORT_C9X)
	add_definitions("-std=c9x")
else (SUPPORT_C9X)
	message(STATUS "Could not enable c9x support")
endif (SUPPORT_C9X)
