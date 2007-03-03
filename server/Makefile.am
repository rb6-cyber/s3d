bin_PROGRAMS=	s3d

if BUILD_GLUT
MAYBE_GLUT=	graphics_glut.c user_glut.c
MAYBE_GLUT_LIBS=	@GLUT_LIBS@
MAYBE_GLUT_CFLAGS=	@GLUT_CFLAGS@
endif
if BUILD_SDL
MAYBE_SDL=	graphics_sdl.c user_sdl.c
MAYBE_SDL_LIBS=	@SDL_LIBS@
MAYBE_SDL_CFLAGS=	@SDL_CFLAGS@
endif

# for s3d_keysym.h
s3d_CPPFLAGS= -I${top_srcdir}/libs3d
s3d_SOURCES= \
	cull.c error.c event.c graphics.c \
	$(MAYBE_GLUT) $(MAYBE_SDL) \
	main.c matrix.c mcp.c navigation.c \
	network.c object.c process.c proto.c \
	shm.c shm_ringbuf.c tcp.c user.c \
	allocate.c

s3d_LDADD=	@LIBM@ @GL_LIBS@ ${MAYBE_SDL_LIBS} ${MAYBE_GLUT_LIBS}
s3d_CFLAGS=	@GL_CPPFLAGS@ ${MAYBE_SDL_CFLAGS} ${MAYBE_GLUT_CFLAGS} ${DEVEL_CFLAGS} 

noinst_HEADERS=	config.h global.h proto.h
