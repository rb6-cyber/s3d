bin_PROGRAMS=	dot_mcp

dot_mcpincludedir = $(pkgincludedir)/dot_mcp
dot_mcpinclude_HEADERS =  

dot_mcp_SOURCES =  \
	main.c menu.c

dot_mcp_CPPFLAGS=	-I$(top_srcdir)/client @PTHREAD_CFLAGS@
dot_mcp_LDADD=	$(top_builddir)/client/libs3d.la @PTHREAD_LIBS@

