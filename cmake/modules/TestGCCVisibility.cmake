include(CheckCCompilerFlag)

# Hide non exported functions/variables
check_c_compiler_flag("-fvisibility=hidden -DHAVE_GCCVISIBILITY" HAVE_GCCVISIBILITY)
if (HAVE_GCCVISIBILITY)
	add_definitions("-fvisibility=hidden -DHAVE_GCCVISIBILITY")
endif (HAVE_GCCVISIBILITY)
