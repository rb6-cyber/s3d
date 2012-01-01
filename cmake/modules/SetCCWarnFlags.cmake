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