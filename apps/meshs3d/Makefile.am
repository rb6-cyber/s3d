bin_PROGRAMS=	meshs3d

meshs3dincludedir = $(pkgincludedir)/meshs3d

meshs3dinclude_HEADERS =  \
	meshs3d.h hash.h
	
meshs3d_SOURCES =  \
	main.c \
	net.c \
	process.c \
	hash.c

meshs3d_CPPFLAGS=	-pg -I$(top_srcdir)/libs3d -I$(top_srcdir)/libs3dw
meshs3d_LDADD=		$(top_builddir)/libs3d/libs3d.la $(top_builddir)/libs3dw/libs3dw.la
meshs3d_LDFLAGS=	-pg
