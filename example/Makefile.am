noinst_PROGRAMS=	hudtest katze \
			ptrtest radius_test s3dclock snowman \
			strtest texturetest linetest widgets
bin_PROGRAMS= filebrowser 3dsloader
			
3dsloader_SOURCES=	3dsloader.c
widgets_SOURCES=	widgets.c
filebrowser_SOURCES=	filebrowser.c
hudtest_SOURCES=	hudtest.c
katze_SOURCES=		katze.c
linetest_SOURCES=	linetest.c
radius_test_SOURCES=	radius_test.c
s3dclock_SOURCES=	s3dclock.c
snowman_SOURCES=	snowman.c
strtest_SOURCES=	strtest.c
texturetest_SOURCES=	texturetest.c
ptrtest_SOURCES=	ptrtest.c

CPPFLAGS=		-I$(top_srcdir)/server -I$(top_srcdir)/libs3d -I$(top_srcdir)/libs3dw
LIBS=			$(top_builddir)/libs3d/libs3d.la $(top_builddir)/libs3dw/libs3dw.la

EXTRA_DIST=	\
	TODO
