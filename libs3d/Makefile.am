lib_LTLIBRARIES=	libs3d.la

libs3d_la_SOURCES=	modelread.c callback.c error.c event.c fontselect.c \
			freetype.c io.c network.c object_queue.c proto_in.c \
			proto_out.c shm.c shm_ringbuf.c tcp.c \
			sei_construct.c sei_misc.c sei_monotone.c sei_tri.c vector.c
			
include_HEADERS= s3d.h s3d_keysym.h
noinst_HEADERS=		config.h s3dlib.h sei_triangulate.h sei_interface.h


libs3d_la_CPPFLAGS=	-I${top_srcdir}/server
#fontconfig_lo_CFLAGS=	@FONTCONFIG_CFLAGS@
#freetype_lo_CFLAGS=	@FREETYPE_CFLAGS@
#modelread_lo_CFLAGS=	@GLIB_CFLAGS@ @G3D_CFLAGS@
libs3d_la_CFLAGS=		@FONTCONFIG_CFLAGS@ @FREETYPE_CFLAGS@ @GLIB_CFLAGS@ @G3D_CFLAGS@
libs3d_la_LIBADD=		@FONTCONFIG_LIBS@ @FREETYPE_LIBS@ @LIBM@ @G3D_LIBS@

EXTRA_DIST=		lgpl.txt
