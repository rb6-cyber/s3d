bin_PROGRAMS=	s3dgps

s3dgps_SOURCES =  \
	s3dgps.c

s3dgps_CPPFLAGS=	-I$(top_srcdir)/libs3d @GPS_CFLAGS@ @SIMAGE_CFLAGS@
s3dgps_LDADD=		$(top_builddir)/libs3d/libs3d.la @GPS_LIBS@ @SIMAGE_LIBS@

