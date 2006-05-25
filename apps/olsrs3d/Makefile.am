bin_PROGRAMS=	olsrs3d

olsrs3dincludedir = $(pkgincludedir)/olsrs3d
olsrs3dinclude_HEADERS =  \
	olsrs3d.h \
	terminal.h

olsrs3d_SOURCES =  \
	process.c \
	main.c \
	net.c \
	terminal.c

olsrs3d_CPPFLAGS=	-I$(top_srcdir)/libs3d
olsrs3d_LDADD=		$(top_builddir)/libs3d/libs3d.la

EXTRA_DIST = \
	output.sample TODO

