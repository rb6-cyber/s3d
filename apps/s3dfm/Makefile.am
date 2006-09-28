bin_PROGRAMS=	s3dfm

s3dfmincludedir = ${pkgincludedir}/s3dfm

s3dfminclude_HEADERS =  \
	s3dfm.h
s3dfm_SOURCES =  \
	main.c box.c animation.c event.c \
	fs.c dialog.c parse.c icon.c node.c \
	string.c fly.c focus.c

s3dfm_CPPFLAGS=		-I${top_srcdir}/libs3d -I${top_srcdir}/libs3dw
s3dfm_LDADD=		${top_builddir}/libs3d/libs3d.la ${top_builddir}/libs3dw/libs3dw.la


