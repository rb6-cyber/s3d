bin_PROGRAMS=	s3dvt

s3dvtincludedir = ${includedir}/s3dvt
s3dvtinclude_HEADERS =  \
	s3dvt.h

s3dvt_SOURCES =  \
	main.c \
	terminal.c

s3dvt_CPPFLAGS=	-I${top_srcdir}/libs3d @PTHREAD_CFLAGS@
s3dvt_LDADD=	${top_builddir}/libs3d/libs3d.la @PTHREAD_LIBS@

