# add option for enabling/disabling profiling
option(PROFILING "Enable/disable support for profiling with gprof" OFF)

if (PROFILING)
	include(CheckCCompilerFlag)

	# check if c compiler understands -pg
	check_c_compiler_flag("-pg" SUPPORT_PG)
	if (SUPPORT_PG)
		add_definitions("-pg")
		list(APPEND CMAKE_EXE_LINKER_FLAGS "-pg")
	else (SUPPORT_PG)
		message(FATAL_ERROR "Compiler doesn't understand -pg")
	endif (SUPPORT_PG)
endif (PROFILING)
