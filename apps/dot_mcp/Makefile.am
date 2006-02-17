bin_PROGRAMS=	dot_mcp

s3dvtincludedir = ${includedir}/dot_mcp
s3dvtinclude_HEADERS =  

s3dvt_SOURCES =  \
	dot_mcp.c

s3dvt_CPPFLAGS=	-I${top_srcdir}/client @PTHREAD_CFLAGS@
s3dvt_LDADD=	${top_builddir}/client/libs3d.la @PTHREAD_LIBS@

