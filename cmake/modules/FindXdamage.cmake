# Try to find Xdamage
#  XDAMAGE_FOUND - If false, do not try to use Xdamage
#  XDAMAGE_LIBRARIES - the libraries to link against
#  XDAMAGE_DEFINITIONS - switches required for Xdamage
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


if (XDAMAGE_LIBRARIES)
	# path set by user or was found in the past
	set(XDAMAGE_FOUND TRUE)
else (XDAMAGE_LIBRARIES)
	set(XDAMAGE_DEFINITIONS "")

	find_library(XDAMAGE_LIBRARIES
		NAMES Xdamage
		PATHS
	)

	if (XDAMAGE_LIBRARIES)
		set(XDAMAGE_FOUND TRUE)
	endif (XDAMAGE_LIBRARIES)

	if (XDAMAGE_FOUND)
		if (NOT XDAMAGE_FIND_QUIETLY)
			message(STATUS "Found Xdamage: ${XDAMAGE_LIBRARIES}")
		endif (NOT XDAMAGE_FIND_QUIETLY)
	else (XDAMAGE_FOUND)
		if (XDAMAGE_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find Xdamage")
		endif (XDAMAGE_FIND_REQUIRED)
	endif (XDAMAGE_FOUND)

	# set visibility in cache
	set(XDAMAGE_DEFINITIONS ${XDAMAGE_DEFINITIONS} CACHE STRING "Defines for compilation." FORCE)
	mark_as_advanced(XDAMAGE_LIBRARIES XDAMAGE_DEFINITIONS)

endif (XDAMAGE_LIBRARIES)
