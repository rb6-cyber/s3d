bin_PROGRAMS=	olsrs3d

olsrs3dincludedir = $(pkgincludedir)/olsrs3d
olsrs3dinclude_HEADERS =  \
	olsrs3d.h

olsrs3d_SOURCES =  \
	vector.c \
	process.c \
	main.c \
	net.c

olsrs3d_CPPFLAGS=	-I$(top_srcdir)/client
olsrs3d_LDADD=		$(top_builddir)/client/libs3d.la

EXTRA_DIST = \
	output.sample TODO

