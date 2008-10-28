# Copyright (c) 2007-2008 Sven Eckelmann <sven.eckelmann@gmx.de>
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

# if user set debug level
string(TOUPPER "${DEBUG}" DEBUG_UPPER)
set(DEBUG ${DEBUG}
	CACHE STRING "Set Debug Level"
)

# test debug and convert it so numeric debug_level
set(DEBUG_LEVEL CACHE INTERNAL "Numeric representation of DEBUG")
if (DEBUG_UPPER STREQUAL VLOW)
	set(DEBUG_LEVEL "1")
elseif (DEBUG_UPPER STREQUAL LOW)
	set(DEBUG_LEVEL "2")
elseif (DEBUG_UPPER STREQUAL MED)
	set(DEBUG_LEVEL "3")
elseif (DEBUG_UPPER STREQUAL HIGH)
	set(DEBUG_LEVEL "4")
elseif (DEBUG_UPPER STREQUAL VHIGH)
	set(DEBUG_LEVEL "5")
elseif (DEBUG_UPPER STREQUAL "")
	set(DEBUG)
else (DEBUG_UPPER STREQUAL VLOW)
	message(FATAL_ERROR "No valid debug-level [VLOW|LOW|MED|HIGH|VHIGH] found.")
endif (DEBUG_UPPER STREQUAL VLOW)

if (DEBUG)
	message(STATUS "Set debug level to: ${DEBUG}")
endif (DEBUG)