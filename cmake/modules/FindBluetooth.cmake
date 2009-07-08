# Try to find Bluetooth
#  BLUETOOTH_FOUND - If false, do not try to use Bluetooth.
#  BLUETOOTH_INCLUDE_DIRS - where to find bluetooth/bluetooth.h
#  BLUETOOTH_LIBRARIES - the libraries to link against
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


if (BLUETOOTH_LIBRARIES AND BLUETOOTH_INCLUDE_DIRS)
	# path set by user or was found in the past
	set(BLUETOOTH_FOUND TRUE)
else (BLUETOOTH_LIBRARIES AND BLUETOOTH_INCLUDE_DIRS)
	find_path(BLUETOOTH_INCLUDE_DIRS
		NAMES bluetooth/bluetooth.h
	)

	find_library(BLUETOOTH_LIBRARIES
		NAMES bluetooth
	)

	if (BLUETOOTH_INCLUDE_DIRS AND BLUETOOTH_LIBRARIES)
		set(BLUETOOTH_FOUND TRUE)
	endif (BLUETOOTH_INCLUDE_DIRS AND BLUETOOTH_LIBRARIES)

	if (BLUETOOTH_FOUND)
		if (NOT BLUETOOTH_FIND_QUIETLY)
			message(STATUS "Found Bluetooth: ${BLUETOOTH_LIBRARIES}")
		endif (NOT BLUETOOTH_FIND_QUIETLY)
	else (BLUETOOTH_FOUND)
		if (BLUETOOTH_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find Bluetooth")
		endif (BLUETOOTH_FIND_REQUIRED)
	endif (BLUETOOTH_FOUND)

	# set visibility in cache
	mark_as_advanced(BLUETOOTH_INCLUDE_DIRS BLUETOOTH_LIBRARIES)

endif (BLUETOOTH_LIBRARIES AND BLUETOOTH_INCLUDE_DIRS)
