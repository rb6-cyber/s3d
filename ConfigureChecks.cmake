include(LateErrors)

# find required lib and add include dir for FREETYPE
find_package(FREETYPE)
if (FREETYPE_FOUND)
	include_directories(${FREETYPE_INCLUDE_DIR})
else (FREETYPE_FOUND)
	PkgError_Later("Could not find FreeType (pkg name: libfreetype6-dev)")
endif (FREETYPE_FOUND)


# find required lib and add include dir for FONTCONFIG
find_package(Fontconfig)
if (FONTCONFIG_FOUND)
	include_directories(${FONTCONFIG_INCLUDE_DIR})
else (FONTCONFIG_FOUND)
	PkgError_Later("Could not find Fontconfig (pkg name: libfontconfig-dev)")
endif (FONTCONFIG_FOUND)


# find required lib and add include dir for GLIB
find_package(GLIB)
if (GLIB2_FOUND)
	include_directories(${GLIB2_INCLUDE_DIR})
	add_definitions(${GLIB2_DEFINITIONS})
else (GLIB2_FOUND)
	PkgError_Later("Could not find GLIB (pkg name: libglib-dev)")
endif (GLIB2_FOUND)


# find required lib and add include dir for G3D
find_package(G3D)
if (G3D_FOUND)
	include_directories(${G3D_INCLUDE_DIR})
	add_definitions(${G3D_DEFINITIONS})
else (G3D_FOUND)
	PkgError_Later("Could not find G3D (pkg name: libg3d-dev)")
endif (G3D_FOUND)


# find required lib and add include dir for OPENGL
find_package(OpenGL)
if (OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIR})
else (OPENGL_FOUND)
	PkgError_Later("Could not find OpenGL libs and headers")
endif (OPENGL_FOUND)


# try to find lib and add include dir for GLUT
find_package(GLUT)
if (GLUT_FOUND)
	include_directories(${GLUT_INCLUDE_DIR})
	set(G_GLUT GLUT_FOUND)
endif (GLUT_FOUND)

# try to find lib and add include dir for SDL
find_package(SDL)
if (SDL_FOUND)
	include_directories(${SDL_INCLUDE_DIR})
	set(G_SDL SDL_FOUND)
endif (SDL_FOUND)

# we need SDL and/or GLUT
if (NOT SDL_FOUND AND NOT GLUT_FOUND)
	PkgError_Later("Could not find SDL (pkg name: libsdl-dev) or GLUT (pkg name: libglut-dev)")
endif (NOT SDL_FOUND AND NOT GLUT_FOUND)


# try to find lib and add include dir for LibXml2
find_package(LibXml2)
if (LIBXML2_FOUND)
	include_directories(${LIBXML2_INCLUDE_DIR})
	add_definitions(${LIBXML2_DEFINITIONS})
endif (LIBXML2_FOUND)


# try to find lib and add include dir for Xtst
find_package(Xtst)
if (XTST_FOUND)
	add_definitions(${XTST_DEFINITIONS})
endif (XTST_FOUND)


# try to find lib and add include dir for GPS
find_package(GPS)
if (GPS_FOUND)
	set(HAVE_GPS GPS_FOUND)
endif (GPS_FOUND)


# try to find lib and add include dir for SQLite3
find_package(SQLite3)
if (SQLITE3_FOUND)
	add_definitions(${SQLITE3_DEFINITIONS})
endif (SQLITE3_FOUND)


# try to find lib and add include dir for PThreads
find_package(PThreads)
if (PTHREADS_FOUND)
	include_directories(${PTHREADS_INCLUDE_DIR})
endif (PTHREADS_FOUND)


# test for shm
include(TestForSHM)
if (HAVE_SHM)
	set(SHM HAVE_SHM)
endif (HAVE_SHM)


# test for signal
include(TestForSIGS)
if (HAVE_SIGS)
	set(SIGS HAVE_SIGS)
endif (HAVE_SIGS)


# test for socket
include(TestForTCP)
if (HAVE_TCP)
	set(TCP HAVE_TCP)
endif (HAVE_TCP)


# test for -fvisibility=hidden
include(TestGCCVisibility)


# link always against math library
link_libraries(m)


# print errors (if found)
Collected_PkgErrors()

# Create config.h and add path to config.h to include search path
configure_file(config.h.cmake ${s3d_BINARY_DIR}/config-s3d.h)
include_directories(${s3d_BINARY_DIR})
