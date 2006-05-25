bin_PROGRAMS=	s3dfm

s3dfmincludedir = ${pkgincludedir}/s3dfm

s3dfminclude_HEADERS =  \
	s3dfm.h
s3dfm_SOURCES =  \
	main.c box.c animation.c

s3dfm_CPPFLAGS=	-I${top_srcdir}/libs3d
s3dfm_LDADD=		${top_builddir}/libs3d/libs3d.la


