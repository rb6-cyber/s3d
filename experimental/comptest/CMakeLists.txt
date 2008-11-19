project(comptest C)

if (X11_FOUND AND XCOMPOSITE_FOUND AND XDAMAGE_FOUND AND XFIXES_FOUND AND XRENDER_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d)

	add_executable(comptest comptest.c window.c x11.c)
	target_link_libraries(comptest ${S3D_LIBRARIES} ${X11_LIBRARIES}
		${XCOMPOSITE_LIBRARIES} ${XFIXES_LIBRARIES} ${XDAMAGE_LIBRARIES}
		${XRENDER_LIBRARIES} ${MATH_LIBRARIES})

	# install
	install(TARGETS comptest RUNTIME DESTINATION "${BIN_INSTALL_DIR}")
else (X11_FOUND AND XCOMPOSITE_FOUND AND XDAMAGE_FOUND AND XFIXES_FOUND AND XRENDER_FOUND)
	if (NOT X11_FOUND)
		PkgStatus_Later("x11 (pkg name: libx11-dev)")
	endif (NOT X11_FOUND)

	if (NOT XCOMPOSITE_FOUND)
		PkgStatus_Later("xcomposite (pkg name: libxcomposite-dev)")
	endif (NOT XCOMPOSITE_FOUND)

	if (NOT XDAMAGE_FOUND)
		PkgStatus_Later("xdamage (pkg name: libxdamage-dev)")
	endif (NOT XDAMAGE_FOUND)

	if (NOT XFIXES_FOUND)
		PkgStatus_Later("xfixes (pkg name: libxfixes-dev)")
	endif (NOT XFIXES_FOUND)

	if (NOT XRENDER_FOUND)
		PkgStatus_Later("xrender (pkg name: libxrender-dev)")
	endif (NOT XRENDER_FOUND)

	# print late status
	Collected_PkgStatus("comptest")
endif (X11_FOUND AND XCOMPOSITE_FOUND AND XDAMAGE_FOUND AND XFIXES_FOUND AND XRENDER_FOUND)
