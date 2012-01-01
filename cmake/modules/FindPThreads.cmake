# Try to find PThreads
#  PTHREADS_FOUND - If false, do not try to use PThreads.
#  PTHREADS_INCLUDE_DIRS - where to find pthreads.h
#  PTHREADS_LIBRARIES - the libraries to link against
#
# Copyright (C) 2007-2012  Sven Eckelmann <sven@narfation.org>
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


if (PTHREADS_LIBRARIES AND PTHREADS_INCLUDE_DIRS)
	# path set by user or was found in the past
	set(PTHREADS_FOUND TRUE)
else (PTHREADS_LIBRARIES AND PTHREADS_INCLUDE_DIRS)
	find_path(PTHREADS_INCLUDE_DIRS
		NAMES pthread.h
	)

	find_library(PTHREADS_LIBRARIES
		NAMES pthread
	)

	if (NOT PTHREADS_LIBRARIES)
		find_library(PTHREADS_LIBRARIES
			NAMES pthreads
		)
	endif (NOT PTHREADS_LIBRARIES)

	if (PTHREADS_INCLUDE_DIRS AND PTHREADS_LIBRARIES)
		set(PTHREADS_FOUND TRUE)
	endif (PTHREADS_INCLUDE_DIRS AND PTHREADS_LIBRARIES)

	if (PTHREADS_FOUND)
		if (NOT PTHREADS_FIND_QUIETLY)
			message(STATUS "Found PThreads: ${PTHREADS_LIBRARIES}")
		endif (NOT PTHREADS_FIND_QUIETLY)
	else (PTHREADS_FOUND)
		if (PTHREADS_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find PThreads")
		endif (PTHREADS_FIND_REQUIRED)
	endif (PTHREADS_FOUND)

	# set visibility in cache
	mark_as_advanced(PTHREADS_INCLUDE_DIRS PTHREADS_LIBRARIES)

endif (PTHREADS_LIBRARIES AND PTHREADS_INCLUDE_DIRS)
