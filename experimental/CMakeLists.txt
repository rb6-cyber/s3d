# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

option(BUILD_EXPERIMENTAL "Enable/disable build of experimental programs" ON)

if (BUILD_EXPERIMENTAL)
	add_subdirectory(comptest)
	add_subdirectory(olsrs3d)
	add_subdirectory(testvis)
	add_subdirectory(wiitest)
endif (BUILD_EXPERIMENTAL)
