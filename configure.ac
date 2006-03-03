dnl Process this file with autoconf to produce a configure script.
AC_INIT(server/main.c)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([s3d], [0.1])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PATH_X
AC_PATH_XTRA
AC_PROG_LIBTOOL
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/time.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([select socket strerror strtod strtol shmget signal]) # XXX: the previous test should have established the existence of signal() already. just be safe here.

WARNS=""
CFLAGS="$CFLAGS $X_CFLAGS"
LIBS="$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS $LIBS"

###
### Optional features 
###
DEVEL_CFLAGS=
DEVEL_LDFLAGS=
# XXX: switch default and help string for releases.
AC_ARG_ENABLE([devel], AS_HELP_STRING([--disable-debug],[Disables debugging flags]), [enable_debug=$enableval], [enable_debug=yes])
if test x$enable_debug = xyes; then
	DEVEL_CFLAGS="$DEVEL_CFLAGS -Wall -pedantic -ansi -ggdb"
	DEVEL_LDFLAGS="$DEVEL_LDFLAGS -ggdb"
fi

AC_ARG_ENABLE([profiling], AS_HELP_STRING([--enable-profiling], [Enable building for profiling]), [enable_profiling=$enableval], [enable_profiling=no])
if test x$enable_profiling = xyes; then
	DEVEL_CFLAGS="$DEVEL_CFLAGS -pg"
	DEVEL_LDFLAGS="$DEVEL_LDFLAGS -pg"
fi

AC_SUBST(DEVEL_CFLAGS)
AC_SUBST(DEVEL_LDFLAGS)
dnl ##################################### SERVER #######################################
AC_CHECK_LIBM
AC_SUBST(LIBM)

MDL_HAVE_OPENGL
AC_SEARCH_LIBS(glutInit, glut, [
HAVE_glut=x
GLUT_CFLAGS=
GLUT_LIBS=-lglut
], [
HAVE_glut=
GLUT_CFLAGS=
GLUT_LIBS=
])
AC_SUBST(GLUT_CFLAGS)
AC_SUBST(GLUT_LIBS)

SDL_VERSION=1.2.7
AM_PATH_SDL($SDL_VERSION, [HAVE_SDL="x"], [])
if test "x${HAVE_glut}" != "x"; then
	AC_DEFINE([G_GLUT], 1, [Use GLUT])
fi
if test "x${HAVE_SDL}" != "x"; then
	AC_DEFINE([G_SDL], 1, [Use SDL])
fi
if test "x${HAVE_glut}${HAVE_SDL}" = "x"; then
	AC_MSG_ERROR([No OpenGL Framework found. Try installing GLUT or SDL, or both])
fi

AM_CONDITIONAL([BUILD_SDL], test x"${HAVE_SDL}" != x)
AM_CONDITIONAL([BUILD_GLUT], test x"${HAVE_glut}" != x)

dnl ##################################### CLIENT #######################################

PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.3.0, [:], [AC_MSG_ERROR([Please install fontconfig (version 2.3.0 or higher)])])
PKG_CHECK_MODULES(FREETYPE, freetype2 >= 9.5.0, [:], [AC_MSG_ERROR([Please install freetype2 (version 9.5.0 or higher)])])

dnl if test "x${HAVE_glut}" = "x"; then
dnl     AC_MSG_ERROR([Sorry, we need glut for the client in for Glyph Tesselation ...])
dnl fi

dnl ##################################### EXAMPLES / APPS ######################

AC_CHECK_LIB(gps, main, [HAVE_GPS=yes], [HAVE_GPS=no])
if test x$HAVE_GPS = xyes; then
	GPS_CFLAGS=
	GPS_LIBS=-lgps	
	AC_SUBST(GPS_CFLAGS)
	AC_SUBST(GPS_LIBS)
else
	WARNS="$WARNS
 ### Sorry, you lack the gps library to compile the s3dgps application."
fi
SIM_AC_HAVE_SIMAGE_IFELSE([
	SIMAGE_CFLAGS=$sim_ac_simage_cppflags
	SIMAGE_LIBS="$sim_ac_simage_ldflags $sim_ac_simage_libs"
	AC_SUBST(SIMAGE_CFLAGS)
	AC_SUBST(SIMAGE_LIBS)
],[
	WARNS="$WARNS
 ### Sorry, you lack the simage library to compile the s3dgps application."
])
AM_CONDITIONAL([BUILD_S3DGPS], [test x$HAVE_GPS = xyes && test x$sim_ac_simage_avail = xtrue])

AC_CHECK_LIB(Xtst, main, [HAVE_Xtst=yes], [HAVE_Xtst=no], [$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS])
if test x$HAVE_Xtst = xyes; then
	XTST_CFLAGS=
	XTST_LIBS="$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS -lXtst"
	AC_SUBST(XTST_CFLAGS)
	AC_SUBST(XTST_LIBS)
else
	WARNS="$WARNS
 ### Sorry, you lack the Xtest Extension to compile the s3d to X11 Gateway."
fi
AM_CONDITIONAL([BUILD_X11GATE], [test x$HAVE_Xtst = xyes])

HAVE_PTHREAD

AC_ARG_WITH([examples], AS_HELP_STRING([--with-examples], [build examples]), [with_examples=$withval], [with_examples=yes])
AM_CONDITIONAL([BUILD_EXAMPLES], [test x$with_examples = xyes])

###
### This part just defined some things in config.h
### XXX: we should provide configure option to disable these parts, or don't use conditional
###      compiles on those symbols (and instead just code parts that either get or don't get 
###      linked in.

if test x${ac_cv_func_socket} = xyes; then
	AC_DEFINE([TCP], 1, [Define if TCP/IP network code should be compiled])
fi
if test x${ac_cv_func_shmget} = xyes; then
	AC_DEFINE([SHM], 1, [Define if shared memory networking code should be compiled])
fi
if test x${ac_cv_func_signal} = xyes; then
	# XXX: win32? we need to figure out if those actually exist on cygwin.
	AC_DEFINE([SIGS], 1, [Define if signal code should be compiled])
fi

AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([
	Makefile
	server/Makefile
	client/Makefile
	apps/Makefile
	apps/olsrs3d/Makefile
	apps/s3d_x11gate/Makefile
	apps/s3dgps/Makefile
	apps/s3dvt/Makefile
	apps/s3dfm/Makefile
	apps/dot_mcp/Makefile
	example/Makefile
	objs/Makefile])
AC_OUTPUT

echo "
--------------------------------------------------------------------------------
$WARNS
--------------------------------------------------------------------------------
"
echo "now use 'make' to compile and 'make install' to install"
