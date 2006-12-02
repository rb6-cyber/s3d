bin_PROGRAMS=	s3dosm
s3dosmincludedir = ${pkgincludedir}/s3dosm
s3dosminclude_HEADERS =  \
	s3dosm.h
				  
s3dosm_SOURCES =  \
	main.c http_fetcher.c http_error_codes.c osm.c \
	object.c draw.c kismet.c nav.c tag.c io.c db.c \
	gps.c

s3dosm_CPPFLAGS=	-pg -I$(top_srcdir)/libs3d @LIBXML2_CFLAGS@ @LIBSQLITE3_CFLAGS@ @GPS_CFLAGS@
s3dosm_LDADD=		$(top_builddir)/libs3d/libs3d.la @LIBXML2_LIBS@ @LIBSQLITE3_LIBS@ @GPS_LIBS@
s3dosm_LDFLAGS=		-pg

