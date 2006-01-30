bin_PROGRAMS=	s3dgps

s3dgps_SOURCES =  \
	s3dgps.c

s3dgps_CPPFLAGS=	-I${top_srcdir}/client @GPS_CFLAGS@
s3dgps_LDADD=		${top_builddir}/client/libs3d.la @GPS_LIBS@

EXTRA_DIST =  \
	arrow.3ds

