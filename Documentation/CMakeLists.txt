add_subdirectory(libs3d)
add_subdirectory(libs3dw)
add_subdirectory(manpages)
add_subdirectory(server)

project(Documentation C)

if (DOCBOOK2HTML_COMPILER)
	add_custom_command(
		OUTPUT  ${Documentation_BINARY_DIR}/doc_output/index.html
		COMMAND ${DOCBOOK2HTML_COMPILER}
		ARGS    ${DOCBOOK2HTML_ARGS} ${Documentation_SOURCE_DIR}/s3d.docbook -o ${Documentation_BINARY_DIR}/doc_output
		DEPENDS ${Documentation_SOURCE_DIR}/s3d.docbook
			${Documentation_SOURCE_DIR}/bookinfo.docbook
			${Documentation_SOURCE_DIR}/server/server.docbook
			${Documentation_SOURCE_DIR}/libs3dw/libs3dw.docbook
			${Documentation_SOURCE_DIR}/libs3d/libs3d.docbook
		COMMENT "Build s3d documentation"
	)

	add_custom_target(documentation ALL echo
		DEPENDS   ${Documentation_BINARY_DIR}/doc_output/index.html
	)

	# install
	set(DOC_DIR ${CMAKE_INSTALL_PREFIX}/share/doc/s3d)
	install(CODE "file(GLOB DOCS ${Documentation_BINARY_DIR}/doc_output/*)\n foreach(file \${DOCS}) \n file(INSTALL DESTINATION ${DOC_DIR} TYPE FILE FILES \"\${file}\") \n endforeach (file \"\${DOCS}\")")
else (DOCBOOK2HTML_COMPILER)
	PkgStatus_Later("xmlto (pkg name: xmlto)")

	# print late status
	Collected_PkgStatus("documentation")
endif (DOCBOOK2HTML_COMPILER)
