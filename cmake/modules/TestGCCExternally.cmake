# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

include(CheckCCompilerFlag)

# Hide non exported functions/variables
check_c_compiler_flag("-fwhole-program -DHAVE_GCCEXTERNALLY" HAVE_GCCEXTERNALLY)
if (HAVE_GCCEXTERNALLY)
	add_definitions("-DHAVE_GCCEXTERNALLY")
endif (HAVE_GCCEXTERNALLY)
