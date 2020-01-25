# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

include(CheckCCompilerFlag)

# put data in new section for linker optimization
check_c_compiler_flag("-fdata-sections" HAVE_GCCDATASECTIONS)
if (HAVE_GCCDATASECTIONS)
	add_definitions("-fdata-sections")
endif (HAVE_GCCDATASECTIONS)

# put function in new section for linker optimization
check_c_compiler_flag("-ffunction-sections" HAVE_GCCFUNCTIONSECTIONS)
if (HAVE_GCCFUNCTIONSECTIONS)
	add_definitions("-ffunction-sections")
endif (HAVE_GCCFUNCTIONSECTIONS)
