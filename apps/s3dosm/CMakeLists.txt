project(s3dosm)

# depends on xml2, sqlite3
if (LIBXML2_FOUND AND SQLITE3_FOUND)
	include_directories(${s3d_SOURCE_DIR}/libs3d ${s3d_SOURCE_DIR}/libs3dw)

	add_executable(s3dosm db.c draw.c gps.c http_error_codes.c http_fetcher.c#
		io.c kismet.c main.c nav.c net.c object.c olsrs3d.c osm.c
		process.c search.c tag.c ui.c)

	target_link_libraries(s3dosm s3d s3dw ${LIBXML2_LIBRARIES} ${SQLITE3_LIBRARIES})

	if (GPS_FOUND)
		target_link_libraries(s3dosm ${GPS_LIBRARIES})
	endif (GPS_FOUND)

	# install 
	install(TARGETS s3dosm RUNTIME DESTINATION bin)
endif (LIBXML2_FOUND AND SQLITE3_FOUND)
