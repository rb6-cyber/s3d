# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@BIN_INSTALL_DIR@
libdir=@LIB_INSTALL_DIR@
includedir=@INCLUDE_INSTALL_DIR@

Name: libs3dw
Version: @VERSION@
Description: Widget library based on libs3d
Requires.private: libs3d
Libs.private: -lm
Libs: -L${libdir} -ls3dw
Cflags: -I${includedir}
