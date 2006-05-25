noinst_PROGRAMS = \
	test_lib

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-I$(top_srcdir)/include \
	@GLIB_CFLAGS@

test_lib_DEPENDENCIES = ../src/libg3d.la
test_lib_SOURCES = test_lib.c
test_lib_LDADD = ../src/libg3d.la -lm @GLIB_LIBS@
