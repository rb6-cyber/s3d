# This module defines a bunch of variables used as locations for install
# directories and for library creation
#
#  BIN_INSTALL_DIR  - the directory where executables will be installed
#  CFG_INSTALL_DIR  - the directory where configuration will be installed
#  LIB_INSTALL_DIR  - the directory where libraries will be installed
#  INCLUDE_INSTALL_DIR  - the directory where header will be installed
#  DATA_INSTALL_DIR - the parent directory where applications can install their data
#  HTML_INSTALL_DIR - the HTML install dir for documentation
#  MAN_INSTALL_DIR  - the man page install dir
#  PKGCFG_INSTALL_DIR  - the pkg-config install dir
#
# Copyright (C) 2007-2008  Sven Eckelmann <sven@narfation.org>
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

if (WIN32)
	set(CFG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/etc" CACHE PATH "The s3d cfg install dir (default prefix/etc/)")
else (WIN32)
	set(CFG_INSTALL_DIR "/etc" CACHE PATH "The s3d cfg install dir (default /etc/)")
endif (WIN32)

set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "The s3d man install dir (default prefix/bin/)")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "The s3d lib install dir (default prefix/lib/)")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The s3d header install dir (default prefix/include/)")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/s3d/objs" CACHE PATH "The s3d objs install dir (default prefix/share/s3d/objs/)")
set(HTML_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/s3d" CACHE PATH "The s3d html documentation install dir (default prefix/share/doc/s3d/)")
set(MAN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE PATH "The s3d man install dir (default prefix/share/man/)")
set(PKGCFG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" CACHE PATH "The s3d pkg-config install dir (default prefix/lib/pkgconfig/)")

# add option for enabling/disabling pseudo 'global' optimisation
option(ENABLE_FINAL "Enable/disable support for 'global' optimisation" OFF)

if (ENABLE_FINAL)
	# test for -fwhole-program
	include(TestGCCExternally)
	# test for -fdata-sections and -ffunction-sections
	include(TestGCCSections)
endif (ENABLE_FINAL)

macro (s3d_add_library _target _type)
	if (ENABLE_FINAL)
		s3d_create_final_include(${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}/final_include.c excluded ${ARGN})
		set(src ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}/final_include.c ${excluded})
	else (ENABLE_FINAL)
		set(src ${ARGN})
	endif (ENABLE_FINAL)

	add_library(${_target} ${_type} ${src})
endmacro (s3d_add_library)

macro (s3d_add_executable _target)
	if (ENABLE_FINAL)
		s3d_create_final_include(${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}/final_include.c excluded ${ARGN})
		set(src ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}/final_include.c ${excluded})
	else (ENABLE_FINAL)
		set(src ${ARGN})
	endif (ENABLE_FINAL)

	add_executable(${_target} ${src})
endmacro (s3d_add_executable)

macro (s3d_create_final_include _output _excluded)
	file(WRITE "${_output}" "/* GENERATED FILE - DO NOT EDIT */\n")
	foreach (_file ${ARGN})
		get_filename_component(_basename "${_file}" ABSOLUTE)
		if ("${_basename}" MATCHES ".+\\.c$")
			file(APPEND "${_output}" "#include \"${_basename}\"\n")
		else ("${_basename}" MATCHES ".+\\.c$")
			 list(APPEND ${_excluded} "${_basename}")
		endif ("${_basename}" MATCHES ".+\\.c$")
	endforeach (_file)
endmacro (s3d_create_final_include)
