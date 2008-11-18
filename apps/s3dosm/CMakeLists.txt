project(s3dosm C)

# depends on xml2, sqlite3
if (LIBXML2_FOUND AND SQLITE3_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

	add_executable(s3dosm db.c draw.c gps.c http_error_codes.c http_fetcher.c#
		io.c kismet.c main.c nav.c object.c osm.c
		tag.c ui.c)

	target_link_libraries(s3dosm s3d s3dw ${MATH_LIBRARIES} ${LIBXML2_LIBRARIES}
	                      ${SQLITE3_LIBRARIES})

	if (GPS_FOUND)
		target_link_libraries(s3dosm ${GPS_LIBRARIES})
	else (GPS_FOUND)
		PkgStatusPartial_Later("GPS support - libgps (pkg name: libgps-dev or gpsd)")
	endif (GPS_FOUND)

	Collected_PkgStatusPartial("s3dosm")
	# install
	install(TARGETS s3dosm RUNTIME DESTINATION bin)
else (LIBXML2_FOUND AND SQLITE3_FOUND)
	if (NOT LIBXML2_FOUND)
		PkgStatus_Later("libxml2 (pkg name: libxml2-dev)")
	endif (NOT LIBXML2_FOUND)

	if (NOT SQLITE3_FOUND)
		PkgStatus_Later("SQLite3 (pkg name: libsqlite3-dev)")
	endif (NOT SQLITE3_FOUND)

	# print late status
	Collected_PkgStatus("s3dosm")
endif (LIBXML2_FOUND AND SQLITE3_FOUND)
