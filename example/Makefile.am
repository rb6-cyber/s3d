noinst_PROGRAMS=	hudtest katze \
			ptrtest radius_test s3dclock snowman \
			strtest texturetest linetest widgets \
			3dsloader_g3d
bin_PROGRAMS= filebrowser 3dsloader
			
3dsloader_SOURCES=		3dsloader.c
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
3dsloader_g3d_SOURCES=	3dsloader_g3d.c

CPPFLAGS=		-I$(top_srcdir)/server -I$(top_srcdir)/libs3d 
LIBS=			$(top_builddir)/libs3d/libs3d.la  
3dsloader_g3d_CFLAGS=	-I$(top_srcdir)/libs3d/libg3d/include @GLIB_CFLAGS@
widgets_CFLAGS=			-I$(top_srcdir)/libs3dw 
widgets_LDFLAGS=		$(top_builddir)/libs3dw/libs3dw.la
3dsloader_g3d_LDFLAGS=	$(top_builddir)/libs3d/libg3d/src/libg3d.la

EXTRA_DIST=	\
	TODO
