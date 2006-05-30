lib_LTLIBRARIES=	libs3dw.la

libs3dw_la_SOURCES=	button.c surface.c object.c style.c event.c animate.c label.c input.c
			
include_HEADERS= 	s3dw.h
noinst_HEADERS=		s3dw_int.h

libs3dw_la_CPPFLAGS=	-I${top_srcdir}/libs3d
libs3dw_la_CFLAGS=	@FONTCONFIG_CFLAGS@ @FREETYPE_CFLAGS@ 
libs3dw_la_LIBADD=	@FONTCONFIG_LIBS@ @FREETYPE_LIBS@ @LIBM@

EXTRA_DIST=		lgpl.txt
