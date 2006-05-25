lib_LTLIBRARIES = libg3d.la

libg3d_la_SOURCES = \
	context.c \
	plugins.c \
	model.c \
	object.c \
	face.c \
	material.c \
	texture.c \
	read.c \
	iff.c \
	primitive.c \
	vector.c \
	matrix.c

INCLUDES = -I$(top_builddir)/include

libg3d_la_CFLAGS = \
	-Wall -ansi -pedantic \
	-fstrict-aliasing -Wstrict-aliasing \
	@DBG_CFLAGS@ \
	@GLIB_CFLAGS@

libg3d_la_LDFLAGS = \
	-version-number "$(subst .,:,@VERSION@)"

libg3d_la_LIBADD = \
	@GLIB_LIBS@
