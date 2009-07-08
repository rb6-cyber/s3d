# Try to find CWiid
#  CWIID_FOUND - If false, do not try to use CWiid.
#  CWIID_INCLUDE_DIRS - where to find cwiid.h
#  CWIID_LIBRARIES - the libraries to link against
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

find_package(Bluetooth)

if (CWIID_LIBRARIES AND CWIID_INCLUDE_DIRS AND BLUETOOTH_FOUND)
	# path set by user or was found in the past
	set(CWIID_FOUND TRUE)
else (CWIID_LIBRARIES AND CWIID_INCLUDE_DIRS AND BLUETOOTH_FOUND)
	find_path(CWIID_INCLUDE_DIRS
		NAMES cwiid.h
	)

	find_library(CWIID_LIBRARY
		NAMES cwiid
	)

	set(CWIID_LIBRARIES ${BLUETOOTH_LIBRARIES} ${CWIID_LIBRARY})

	if (CWIID_INCLUDE_DIRS AND CWIID_LIBRARIES AND BLUETOOTH_FOUND)
		set(CWIID_FOUND TRUE)
	endif (CWIID_INCLUDE_DIRS AND CWIID_LIBRARIES AND BLUETOOTH_FOUND)

	if (CWIID_FOUND)
		if (NOT CWIID_FIND_QUIETLY)
			message(STATUS "Found CWiid: ${CWIID_LIBRARIES}")
		endif (NOT CWIID_FIND_QUIETLY)
	else (CWIID_FOUND)
		if (CWIID_FIND_REQUIRED)
			if (NOT BLUETOOTH_FOUND)
				message(FATAL_ERROR "Could not find dependency Bluetooth for CWiid")
			else (NOT BLUETOOTH_FOUND)
				message(FATAL_ERROR "Could not find CWiid")
			endif (NOT BLUETOOTH_FOUND)
		endif (CWIID_FIND_REQUIRED)
	endif (CWIID_FOUND)

	# set visibility in cache
	mark_as_advanced(CWIID_INCLUDE_DIRS CWIID_LIBRARY CWIID_LIBRARIES)

endif (CWIID_LIBRARIES AND CWIID_INCLUDE_DIRS AND BLUETOOTH_FOUND)
