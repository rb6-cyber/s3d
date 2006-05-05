bin_PROGRAMS=	s3dfm

s3dfmincludedir = ${pkgincludedir}/s3dfm

s3dfminclude_HEADERS =  \
	s3dfm.h
s3dfm_SOURCES =  \
	main.c box.c animation.c

s3dfm_CPPFLAGS=	-I${top_srcdir}/client
s3dfm_LDADD=		${top_builddir}/client/libs3d.la


