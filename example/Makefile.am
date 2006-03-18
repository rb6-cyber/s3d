noinst_PROGRAMS=	3dsloader filebrowser hudtest katze \
			ptrtest radius_test s3dclock snowman \
			strtest texturetest linetest
			
3dsloader_SOURCES=	3dsloader.c
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

CPPFLAGS=		-I$(top_srcdir)/server -I$(top_srcdir)/client
LIBS=			$(top_builddir)/client/libs3d.la

EXTRA_DIST=	\
	TODO
