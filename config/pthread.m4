dnl Check for pthread library.
dnl Taken from gnome. This check is okay, but not necessarily portable to 
dnl older systems or systems where the threading library has a strange name
dnl (early FreeBSD 5.x used -lkse, for example.)

AC_DEFUN([HAVE_PTHREAD], [
	PTHREAD_CFLAGS=""
        PTHREAD_LIBS=""
        AC_CHECK_LIB(pthread, pthread_create, PTHREAD_LIBS="-lpthread",
                [AC_CHECK_LIB(pthreads, pthread_create, PTHREAD_LIBS="-lpthreads",
                    [AC_CHECK_LIB(c_r, pthread_create, PTHREAD_LIBS="-lc_r",
                        [AC_CHECK_LIB(pthread, __pthread_attr_init_system, PTHREAD_LIBS="-lpthread",
                                [AC_CHECK_FUNC(pthread_create)]
                        )]
                    )]
                )]
        )
        AC_SUBST(PTHREAD_CFLAGS)
        AC_SUBST(PTHREAD_LIBS)
])
