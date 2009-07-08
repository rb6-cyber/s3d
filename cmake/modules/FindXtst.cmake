# Try to find Xtst
#  XTST_FOUND - If false, do not try to use Xtst.
#  XTST_LIBRARIES - the libraries to link against
#  XTST_DEFINITIONS - switches required for Xtst
#
# Copyright (C) 2007-2008  Sven Eckelmann <sven.eckelmann@gmx.de>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The names of Kitware, Inc., the Insight Consortium, or the names of
#    any consortium members, or of any contributors, may not be used to
#    endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.


if (XTST_LIBRARIES)
	# path set by user or was found in the past
	set(XTST_FOUND TRUE)
else (XTST_LIBRARIES)
	set(XTST_DEFINITIONS "")

	find_library(XTST_LIBRARIES
		NAMES Xtst
		PATHS
	)

	if (XTST_LIBRARIES)
		set(XTST_FOUND TRUE)
	endif (XTST_LIBRARIES)

	if (XTST_FOUND)
		if (NOT XTST_FIND_QUIETLY)
			message(STATUS "Found Xtst: ${XTST_LIBRARIES}")
		endif (NOT XTST_FIND_QUIETLY)
	else (XTST_FOUND)
		if (XTST_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find Xtst")
		endif (XTST_FIND_REQUIRED)
	endif (XTST_FOUND)

	# set visibility in cache
	set(XTST_DEFINITIONS ${XTST_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(XTST_LIBRARIES XTST_DEFINITIONS)

endif (XTST_LIBRARIES)
