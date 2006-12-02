dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.5)
AC_INIT([s3d], [0.1.1])
AC_CONFIG_SRCDIR(server/main.c)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([dist-bzip2])

# XXXHACK: lame attempt to avoid fortran in libtool and elsewhere
tagnames=`echo $tagnames | sed s,F77,,g | sed s,FC,,g` # for AC_PROG_LIBTOOL
# we could skip the test here, as we don't USE F77/FC anywhere, but, the user may
# have a reason. 
if test x${F77} = x; then
	F77=nonexistent
fi
if test x${FC} = x; then
	FC=nonexistent
fi

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
CFLAGS="$CFLAGS $X_CFLAGS -Wall -pedantic"
LIBS="$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS $LIBS"

###
### Optional features 
###
DEVEL_CFLAGS=
DEVEL_LDFLAGS=
# XXX: switch default and help string for releases.
AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug],[Enables debugging flags]), [enable_debug=$enableval], [enable_debug=no])
if test x$enable_debug = xyes; then
	DEVEL_CFLAGS="$DEVEL_CFLAGS -ggdb -DDEBUG=LOW"
	DEVEL_LDFLAGS="$DEVEL_LDFLAGS -ggdb"
fi

AC_ARG_ENABLE([profiling], AS_HELP_STRING([--enable-profiling], [Enable building for profiling]), [enable_profiling=$enableval], [enable_profiling=no])
if test x$enable_profiling = xyes; then
	DEVEL_CFLAGS="$DEVEL_CFLAGS -pg"
	DEVEL_LDFLAGS="$DEVEL_LDFLAGS -pg"
fi

CFLAGS="$CFLAGS $DEVEL_CFLAGS"
LDFLAGS="$LDFLAGS $DEVEL_LDFLAGS"
dnl AC_SUBST(DEVEL_CFLAGS)
dnl AC_SUBST(DEVEL_LDFLAGS)
dnl ##################################### SERVER #######################################
AC_CHECK_LIBM
AC_SUBST(LIBM)

MDL_HAVE_OPENGL
AC_SEARCH_LIBS(glutInit, glut, [
HAVE_glut="yes"
GLUT_CFLAGS=
GLUT_LIBS="-lglut"
], [
HAVE_glut=
GLUT_CFLAGS=
GLUT_LIBS=
])
AC_SUBST(GLUT_CFLAGS)
AC_SUBST(GLUT_LIBS)

SDL_VERSION=1.2.7
AM_PATH_SDL($SDL_VERSION, [HAVE_SDL="yes"], [])
if test "x${HAVE_glut}" = "xyes"; then
	AC_DEFINE([G_GLUT], 1, [Use GLUT])
fi
if test "x${HAVE_SDL}" = "xyes"; then
	AC_DEFINE([G_SDL], 1, [Use SDL])
fi
if test "x${HAVE_SDL}" = "x"; then
	AC_MSG_ERROR([Please install SDL.])
fi

AM_CONDITIONAL([BUILD_SDL], test x"${HAVE_SDL}" = xyes)
AM_CONDITIONAL([BUILD_GLUT], test x"${HAVE_glut}" = xyes)

dnl ##################################### LIBS3D #######################################

PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.1.0, [:], [AC_MSG_ERROR([Please install fontconfig (version 2.1.0 or higher)])])
PKG_CHECK_MODULES(FREETYPE, freetype2 >= 9.5.0, [:], [AC_MSG_ERROR([Please install freetype2 (version 9.5.0 or higher)])])

PKG_CHECK_MODULES(SQLITE3, sqlite3 >= 3.0, [HAVE_SQLITE3="yes"], [HAVE_SQLITE3="no"])
# libxml2
AM_PATH_XML2(2.0.0, [HAVE_XML="yes"], [])

if test x"${HAVE_XML}" = xyes ; then
	LIBXML2_CFLAGS=$XML_CPPFLAGS
	LIBXML2_LIBS=$XML_LIBS
	AC_SUBST(LIBXML2_CFLAGS)
	AC_SUBST(LIBXML2_LIBS)
else
dnl ##################################### EXAMPLES / APPS ######################
	WARNS="$WARNS
 ### Sorry, you lack the libxml2 library to compile the s3dosm application."

fi

	
if test x"${HAVE_SQLITE3}" = xyes ; then
	LIBSQLITE3_CFLAGS=""
	LIBSQLITE3_LIBS="-lsqlite3"
	AC_SUBST(LIBSQLITE3_CFLAGS)
	AC_SUBST(LIBSQLITE3_LIBS)
else
dnl ##################################### EXAMPLES / APPS ######################
	WARNS="$WARNS
 ### Sorry, you lack the sqlite3 library to compile the s3dosm application."

fi



AM_CONDITIONAL([BUILD_S3DOSM], [test x"${HAVE_XML}" = xyes && test x"${HAVE_SQLITE3}" = xyes])

SIM_AC_HAVE_SIMAGE_IFELSE([
	SIMAGE_CFLAGS=$sim_ac_simage_cppflags
	SIMAGE_LIBS="$sim_ac_simage_ldflags $sim_ac_simage_libs"
	AC_SUBST(SIMAGE_CFLAGS)
	AC_SUBST(SIMAGE_LIBS)
],[
	WARNS="$WARNS
 ### Sorry, you lack the simage library to compile the s3dgps application."
])
AC_CHECK_HEADER(gps.h, [HAVE_GPS=yes])
AM_CONDITIONAL([BUILD_S3DGPS], [test x$HAVE_GPS = xyes && test x$sim_ac_simage_avail = xtrue])

if test "x${HAVE_GPS}" = "xyes"; then
	AC_DEFINE([HAVE_GPS], 1, [Have gps support])
	GPS_CFLAGS=
	GPS_LIBS="-lgps"
	AC_SUBST(GPS_CFLAGS)
	AC_SUBST(GPS_LIBS)

fi

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
########################################### LIBG3D ################
AM_PROG_LEX

## gtk-doc
# GTK_DOC_CHECK

# glib
AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR([glib >= 2.0 is needed]), gmodule gobject)

# gdk-pixbuf
#AM_PATH_GTK_2_0(2.0.0, have_gtk2=true)
#if test "x$have_gtk2" = "xtrue" ; then
#GDKPIXBUF_CFLAGS=$GTK_CFLAGS
#GDKPIXBUF_LIBS=$GTK_LIBS
#AC_SUBST(GDKPIXBUF_CFLAGS)
#AC_SUBST(GDKPIXBUF_LIBS)
#fi
#AM_CONDITIONAL([HAVE_GDKPIXBUF], test "x$have_gtk2" = "xtrue")

# AC_DEFINE_DIR(PLUGIN_DIR, libdir/$PACKAGE/plugins, [Plugin directory])

AC_OUTPUT(
)

DBG_CFLAGS=""
AC_SUBST(DBG_CFLAGS)
DEBUG=0
AC_SUBST(DEBUG)
AC_DEFINE_DIR(PLUGIN_DIR, libdir/$PACKAGE/plugins, [Plugin directory])
PLUGINS_LDFLAGS="-avoid-version -export-dynamic"
AC_SUBST(PLUGINS_LDFLAGS)

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

AC_CONFIG_HEADERS(libs3d/libg3d/include/g3d/config.h config.h)
AC_CONFIG_FILES([
	Makefile
	server/Makefile
	libs3d/Makefile
	libs3dw/Makefile
	apps/Makefile
	apps/olsrs3d/Makefile
	apps/s3d_x11gate/Makefile
	apps/s3dgps/Makefile
	apps/s3dvt/Makefile
	apps/s3dfm/Makefile
	apps/kism3d/Makefile
	apps/s3dosm/Makefile
	apps/dot_mcp/Makefile
	example/Makefile
	objs/Makefile
	Documentation/Makefile
	libs3d/libg3d/Makefile
	libs3d/libg3d/include/Makefile
	libs3d/libg3d/include/g3d/Makefile
	libs3d/libg3d/plugins/Makefile
	libs3d/libg3d/plugins/image/Makefile
	libs3d/libg3d/plugins/import/Makefile
	libs3d/libg3d/src/Makefile
	libs3d/libg3d/libg3d.pc
	s3drc])
AC_OUTPUT
# libg3d
#	libs3d/libg3d/doc/Makefile
#	libs3d/libg3d/doc/api/Makefile
#	libs3d/libg3d/m4/Makefile
#	libs3d/libg3d/tests/Makefile
if test "x$WARNS" != x; then
echo "
--------------------------------------------------------------------------------
Warnings (NOT FATAL, safe to ignore if you don't need any of those apps):
$WARNS
--------------------------------------------------------------------------------
"
fi

echo "now use 'make' to compile and 'make install' to install"
