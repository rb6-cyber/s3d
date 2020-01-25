# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

include(CheckCCompilerFlag)

# Enable all warnings if possible (gnu cc)
check_c_compiler_flag("-Wall" SUPPORT_WALL)
if (SUPPORT_WALL)
	add_definitions("-Wall")
endif (SUPPORT_WALL)

# Enable even more warnings if possible (gnu cc)
check_c_compiler_flag("-Wextra" SUPPORT_WEXTRA)
if (SUPPORT_WEXTRA)
	add_definitions("-Wextra")
else (SUPPORT_WEXTRA)
	# try to enable extra warnings on old gcc versions
	check_c_compiler_flag("-W" SUPPORT_WEXTRAOLD)
	if (SUPPORT_WEXTRAOLD)
		add_definitions("-W")
	endif (SUPPORT_WEXTRAOLD)
endif (SUPPORT_WEXTRA)

# Enable pedantic mode if possible (gnu cc)
check_c_compiler_flag("-pedantic" SUPPORT_PEDANTIC)
if (SUPPORT_PEDANTIC)
	add_definitions("-pedantic")
endif (SUPPORT_PEDANTIC)
