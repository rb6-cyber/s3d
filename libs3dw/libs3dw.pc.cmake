prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@/bin
libdir=@CMAKE_INSTALL_PREFIX@/lib
includedir=@CMAKE_INSTALL_PREFIX@/include

Name: libs3dw
Version: @VERSION@
Description: Widget library based on libs3d
Requires.private: libs3d
Libs.private: -lm
Libs: -L${libdir} -ls3dw
Cflags: -I${includedir}
