bin_PROGRAMS=	olsrs3d

olsrs3dincludedir = $(pkgincludedir)/olsrs3d
olsrs3dinclude_HEADERS =  \
	olsrs3d.h \
	mod_search.h

olsrs3d_SOURCES =  \
	process.c \
	main.c \
	net.c \
	mod_search.c

olsrs3d_CPPFLAGS=	-I$(top_srcdir)/libs3d -I$(top_srcdir)/libs3dw
olsrs3d_LDADD=		$(top_builddir)/libs3d/libs3d.la $(top_builddir)/libs3dw/libs3dw.la

EXTRA_DIST = \
	output.sample TODO

