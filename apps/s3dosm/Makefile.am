bin_PROGRAMS=	s3dosm
s3dosmincludedir = ${pkgincludedir}/s3dosm
s3dosminclude_HEADERS =  \
	s3dosm.h
				  
s3dosm_SOURCES =  \
	main.c http_fetcher.c http_error_codes.c osm.c \
	avl.c object.c draw.c kismet.c nav.c tag.c io.c

s3dosm_CPPFLAGS=	-I$(top_srcdir)/libs3d @LIBXML2_CFLAGS@
s3dosm_LDADD=		$(top_builddir)/libs3d/libs3d.la @LIBXML2_LIBS@

