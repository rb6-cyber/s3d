pluginimpdir = $(pkglibdir)/plugins/import

pluginimp_LTLIBRARIES = \
	imp_3dmf.la \
	imp_3ds.la \
	imp_ac3d.la \
	imp_cob.la \
	imp_dxf.la \
	imp_flt.la \
	imp_heightfield.la \
	imp_iob.la \
	imp_leocad.la \
	imp_lwo.la \
	imp_md2.la \
	imp_md3.la \
	imp_nff.la \
	imp_obj.la \
	imp_q3o.la \
	imp_vrml.la

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@

INCLUDES = -I$(top_builddir)/include

AM_LDFLAGS = \
	-module \
	@PLUGINS_LDFLAGS@

AM_LFLAGS = -o${LEX_OUTPUT_ROOT}.c

LIBADD = \
	@GLIB_LIBS@

LIBTOOL += --quiet

imp_3dmf_la_SOURCES = imp_3dmf.c
imp_3ds_la_SOURCES = imp_3ds.c
imp_ac3d_la_SOURCES = imp_ac3d.c
imp_cob_la_SOURCES = imp_cob.c
imp_dxf_la_SOURCES = imp_dxf.c
imp_flt_la_SOURCES = imp_flt.c
imp_heightfield_la_SOURCES = imp_heightfield.c
imp_iob_la_SOURCES = imp_iob.c
imp_leocad_la_SOURCES = imp_leocad.c imp_leocad_library.h imp_leocad_library.c
imp_lwo_la_SOURCES = imp_lwo.c
imp_md2_la_SOURCES = imp_md2.c imp_md2_normals.h
imp_md3_la_SOURCES = imp_md3.c
imp_nff_la_SOURCES = imp_nff.c
imp_obj_la_SOURCES = imp_obj.c
imp_q3o_la_SOURCES = imp_q3o.c
imp_vrml_la_SOURCES = imp_vrml_v1.l imp_vrml.c
imp_vrml_la_LIBADD = ${LEXLIB}

# by default imp_vrml_v1.c seems to be included in package, so add .h too
# also removes strong dependancy on flex
EXTRA_DIST = imp_vrml_v1.c imp_vrml_v1.h
