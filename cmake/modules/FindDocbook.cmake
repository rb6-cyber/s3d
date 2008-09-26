# - Find Docbook
# This module finds if Docbook is installed and determines where the
# executables are. This code sets the following variables:
#
#  DOCBOOK2HTML_COMPILER: path to the HTML convert
#

find_program(DOCBOOK2HTML_COMPILER
	NAMES xmlto
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)
set(DOCBOOK2HTML_ARGS "xhtml")

find_program(DOCBOOK2PDF_COMPILER
	NAMES xmlto
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)
set(DOCBOOK2PDF_ARGS "pdf")

find_program(DOCBOOK2PS_COMPILER
	NAMES xmlto
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)
set(DOCBOOK2PS_ARGS "ps")

mark_as_advanced(
	DOCBOOK2HTML_COMPILER
	DOCBOOK2PDF_COMPILER
	DOCBOOK2PS_COMPILER
)
