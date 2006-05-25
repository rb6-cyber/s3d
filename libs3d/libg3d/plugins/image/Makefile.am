pluginimgdir = $(pkglibdir)/plugins/image

pluginimg_LTLIBRARIES = \
	img_bmp.la \
	img_sgi.la

if HAVE_GDKPIXBUF
pluginimg_LTLIBRARIES += img_gdkpixbuf.la
endif

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@

INCLUDES = -I$(top_builddir)/include

AM_LDFLAGS = \
	-module \
	@PLUGINS_LDFLAGS@

LIBADD = \
	@GLIB_LIBS@

LIBTOOL += --quiet

img_bmp_la_SOURCES = img_bmp.c
img_sgi_la_SOURCES = img_sgi.c

img_gdkpixbuf_la_CFLAGS = ${AM_CFLAGS} @GDKPIXBUF_CFLAGS@
img_gdkpixbuf_la_SOURCES = img_gdkpixbuf.c
img_gdkpixbuf_la_LIBADD = @GDKPIXBUF_LIBS@

