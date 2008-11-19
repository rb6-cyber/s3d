# This module defines a bunch of variables used as locations for install directories.
#
#  BIN_INSTALL_DIR  - the directory where executables will be installed
#  CFG_INSTALL_DIR  - the directory where executables will be installed
#  LIB_INSTALL_DIR  - the directory where libraries will be installed
#  INCLUDE_INSTALL_DIR  - the directory where header will be installed
#  DATA_INSTALL_DIR - the parent directory where applications can install their data
#  HTML_INSTALL_DIR - the HTML install dir for documentation
#  MAN_INSTALL_DIR  - the man page install dir
#  PKGCFG_INSTALL_DIR  - the pkg-config install dir

if (WIN32)
	set(CFG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/etc" CACHE PATH "The s3d cfg install dir (default prefix/etc/)")
	set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "The s3d lib install dir (default prefix/bin/)")
else (WIN32)
	set(CFG_INSTALL_DIR "/etc" CACHE PATH "The s3d cfg install dir (default /etc/)")
	set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "The s3d lib install dir (default prefix/lib/)")
endif (WIN32)

set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "The s3d man install dir (default prefix/bin/)")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The s3d header install dir (default prefix/include/)")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/objs" CACHE PATH "The s3d objs install dir (default prefix/share/objs/)")
set(HTML_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/s3d" CACHE PATH "The s3d html documentation install dir (default prefix/share/doc/s3d/)")
set(MAN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE PATH "The s3d man install dir (default prefix/share/man/)")
set(PKGCFG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" CACHE PATH "The s3d pkg-config install dir (default prefix/lib/pkgconfig/)")
