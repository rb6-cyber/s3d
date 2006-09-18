noinst_PROGRAMS=	hudtest katze \
			ptrtest radius_test s3dclock snowman \
			strtest texturetest linetest widgets \
			nichtsnutz
bin_PROGRAMS= filebrowser modelloader
			
modelloader_SOURCES=	modelloader.c
widgets_SOURCES=		widgets.c
filebrowser_SOURCES=	filebrowser.c
hudtest_SOURCES=		hudtest.c
katze_SOURCES=			katze.c
linetest_SOURCES=		linetest.c
radius_test_SOURCES=	radius_test.c
s3dclock_SOURCES=		s3dclock.c
snowman_SOURCES=		snowman.c
strtest_SOURCES=		strtest.c
texturetest_SOURCES=	texturetest.c
ptrtest_SOURCES=		ptrtest.c
nichtsnutz_SOURCES=		nichtsnutz.c

CPPFLAGS=		-I$(top_srcdir)/server -I$(top_srcdir)/libs3d 
LIBS=			$(top_builddir)/libs3d/libs3d.la  
widgets_CFLAGS=			-I$(top_srcdir)/libs3dw 
widgets_LDFLAGS=		$(top_builddir)/libs3dw/libs3dw.la

EXTRA_DIST=	\
	TODO
