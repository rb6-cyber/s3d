prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@/bin
libdir=@CMAKE_INSTALL_PREFIX@/lib
includedir=@CMAKE_INSTALL_PREFIX@/include

Name: libs3d
Version: @VERSION@
Description: Client library for the s3d server
Requires: libg3d freetype2 fontconfig sdl
Libs: -L${libdir} -ls3d
Cflags: -I${includedir}
