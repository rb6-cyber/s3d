# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@BIN_INSTALL_DIR@
libdir=@LIB_INSTALL_DIR@
includedir=@INCLUDE_INSTALL_DIR@

Name: libs3d
Version: @VERSION@
Description: Client library for the s3d server
Requires.private: libg3d freetype2 fontconfig
Libs.private: -lm
Libs: -L${libdir} -ls3d
Cflags: -I${includedir}
