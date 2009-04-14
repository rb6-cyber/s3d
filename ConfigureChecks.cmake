include(LateErrors)
include(LateStatus)
include(LateStatusPartial)

# find required lib and add include dir for FREETYPE
find_package(FREETYPE)
if (FREETYPE_FOUND)
	include_directories(${FREETYPE_INCLUDE_DIRS})
else (FREETYPE_FOUND)
	PkgError_Later("Could not find FreeType (pkg name: libfreetype6-dev)")
endif (FREETYPE_FOUND)


# find required lib and add include dir for FONTCONFIG
find_package(Fontconfig)
if (FONTCONFIG_FOUND)
	include_directories(${FONTCONFIG_INCLUDE_DIRS})
else (FONTCONFIG_FOUND)
	PkgError_Later("Could not find Fontconfig (pkg name: libfontconfig-dev)")
endif (FONTCONFIG_FOUND)


# find required lib and add include dir for GLIB
find_package(GLIB)
if (GLIB2_FOUND)
	include_directories(${GLIB2_INCLUDE_DIRS})
	add_definitions(${GLIB2_DEFINITIONS})
else (GLIB2_FOUND)
	PkgError_Later("Could not find GLIB2 (pkg name: libglib2.0-dev)")
endif (GLIB2_FOUND)


# find required lib and add include dir for G3D
find_package(G3D)
if (G3D_FOUND)
	include_directories(${G3D_INCLUDE_DIRS})
	add_definitions(${G3D_DEFINITIONS})
else (G3D_FOUND)
	PkgError_Later("Could not find G3D (pkg name: libg3d-dev)")
endif (G3D_FOUND)


# find required lib and add include dir for G3D
find_package(Math)
if (MATH_FOUND)
else (MATH_FOUND)
	PkgError_Later("Could not find libm")
endif (MATH_FOUND)


# find required lib and add include dir for OPENGL
find_package(OpenGL)
if (OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIR})
else (OPENGL_FOUND)
	PkgError_Later("Could not find OpenGL libs and headers")
endif (OPENGL_FOUND)


# try to find lib and add include dir for SDL
find_package(SDL)
if (SDL_FOUND)
	include_directories(${SDL_INCLUDE_DIR})
	set(G_SDL SDL_FOUND)
else (SDL_FOUND)
	PkgError_Later("Could not find SDL (pkg name: libsdl-dev)")
endif (SDL_FOUND)


# find lib and add include dir for CWiid
find_package(CWiid)
if (CWIID_FOUND)
	include_directories(${CWIID_INCLUDE_DIRS})
endif (CWIID_FOUND)


# try to find docbook
find_package(Docbook)


# try to find lib and add include dir for GPS
find_package(GPS)
if (GPS_FOUND)
	set(HAVE_GPS GPS_FOUND)
	if (GPS_NEW_STRUCT)
		set(HAVE_GPS_NEW GPS_NEW_STRUCT)
	endif  (GPS_NEW_STRUCT)
endif (GPS_FOUND)


# try to find lib and add include dir for PThreads
find_package(PThreads)
if (PTHREADS_FOUND)
	include_directories(${PTHREADS_INCLUDE_DIRS})
endif (PTHREADS_FOUND)


# try to find lib and add include dir for SQLite3
find_package(SQLite3)
if (SQLITE3_FOUND)
	include_directories(${SQLITE3_INCLUDE_DIRS})
	add_definitions(${SQLITE3_DEFINITIONS})
endif (SQLITE3_FOUND)


# try to find lib and add include dir for LibXml2
find_package(LibXml2)
if (LIBXML2_FOUND)
	include_directories(${LIBXML2_INCLUDE_DIR})
	add_definitions(${LIBXML2_DEFINITIONS})
endif (LIBXML2_FOUND)


# try to find lib and add include dir for Xcomposite
find_package(Xcomposite)
if (XCOMPOSITE_FOUND)
	include_directories(${XCOMPOSITE_INCLUDE_DIRS})
	add_definitions(${XCOMPOSITE_DEFINITIONS})
endif (XCOMPOSITE_FOUND)

# try to find lib and add include dir for Xdamage
find_package(Xdamage)
if (XDAMAGE_FOUND)
	include_directories(${XDAMAGE_INCLUDE_DIRS})
	add_definitions(${XDAMAGE_DEFINITIONS})
endif (XDAMAGE_FOUND)

# try to find lib and add include dir for Xfixes
find_package(Xfixes)
if (XFIXES_FOUND)
	include_directories(${XFIXES_INCLUDE_DIRS})
	add_definitions(${XFIXES_DEFINITIONS})
endif (XFIXES_FOUND)

# try to find lib and add include dir for Xrender
find_package(Xrender)
if (XRENDER_FOUND)
	include_directories(${XRENDER_INCLUDE_DIRS})
	add_definitions(${XRENDER_DEFINITIONS})
endif (XRENDER_FOUND)

# try to find lib and add include dir for Xtst
find_package(Xtst)
if (XTST_FOUND)
	include_directories(${XTST_INCLUDE_DIRS})
	add_definitions(${XTST_DEFINITIONS})
endif (XTST_FOUND)

# try to find lib and add include dir for Xtst
find_package(X11)
if (X11_FOUND)
	include_directories(${X11_INCLUDE_DIRS})
	add_definitions(${X11_DEFINITIONS})
endif (X11_FOUND)


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

# test how to mark parameter as explicit unused
include(TestUnusedParam)


# print errors (if found)
Collected_PkgErrors()

# Create config.h and add path to config.h to include search path
configure_file(config.h.cmake ${s3d_BINARY_DIR}/config-s3d.h)
include_directories(${s3d_BINARY_DIR})
