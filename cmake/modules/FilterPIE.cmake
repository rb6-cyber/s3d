# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

string(REGEX REPLACE "(-fpie)|(-fPIE)|(-pie)" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
#string(REGEX REPLACE "(-fpie)|(-fPIE)|(-pie)" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "(-fpie)|(-fPIE)|(-pie)" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
