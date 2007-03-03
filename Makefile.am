
if BUILD_EXAMPLES
MAYBE_EXAMPLES=example
else
MAYBE_EXAMPLES=
endif

SUBDIRS=server libs3d libs3dw apps $(MAYBE_EXAMPLES) objs Documentation

EXTRA_DIST=	gpl.txt \
		config/libtool15.m4 config/mdl_have_opengl.m4 \
		config/pkg.m4 config/pthread.m4 config/sdl.m4 

dist_sysconf_DATA=	s3drc

