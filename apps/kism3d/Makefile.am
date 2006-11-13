bin_PROGRAMS=	kism3d

kism3dincludedir = ${pkgincludedir}/kism3d

kism3dinclude_HEADERS =  \
	kism3d.h list.h
kism3d_SOURCES =  \
	kism3d.c

kism3d_CPPFLAGS=		-I${top_srcdir}/libs3d -I${top_srcdir}/libs3dw
kism3d_LDADD=		${top_builddir}/libs3d/libs3d.la ${top_builddir}/libs3dw/libs3dw.la


