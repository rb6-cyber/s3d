# - Find Docbook
# This module finds if Docbook is installed and determines where the
# executables are. This code sets the following variables:
#
#  DOCBOOK2DVI_COMPILER: path to the DVI convert
#  DOCBOOK2HTML_COMPILER: path to the HTML convert
#  DOCBOOK2MAN_COMPILER: path to the MAN convert
#  DOCBOOK2PDF_COMPILER: path to the PDF convert
#  DOCBOOK2PS_COMPILER: path to the PS convert
#  DOCBOOK2RTF_COMPILER: path to the RTF convert
#  DOCBOOK2TEX_COMPILER: path to the TEX convert
#  DOCBOOK2TEXI_COMPILER: path to the TEXI convert
#  DOCBOOK2TXT_COMPILER: path to the TXT convert
#

set(DOCBOOK_BINARY_PATH ${DOCBOOK_BINARY_PATH}
	CACHE PATH "Path to the Docbook binary directory."
)

find_program(DOCBOOK2DVI_COMPILER
	NAMES docbook2dvi
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

find_program(DOCBOOK2HTML_COMPILER
	NAMES docbook2html
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

find_program(DOCBOOK2MAN_COMPILER
	NAMES docbook2man
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

find_program(DOCBOOK2PDF_COMPILER
	NAMES docbook2pdf
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

find_program(DOCBOOK2PS_COMPILER
	NAMES docbook2ps
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

find_program(DOCBOOK2RTF_COMPILER
	NAMES docbook2rtf
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

find_program(DOCBOOK2TEX_COMPILER
	NAMES docbook2tex
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

find_program(DOCBOOK2TEXI_COMPILER
	NAMES docbook2texi
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

find_program(DOCBOOK2TXT_COMPILER
	NAMES docbook2txt
	PATHS ${DOCBOOK_BINARY_PATH}
		/usr/bin
)

mark_as_advanced(
	DOCBOOK2DVI_COMPILER
	DOCBOOK2HTML_COMPILER
	DOCBOOK2MAN_COMPILER
	DOCBOOK2PDF_COMPILER
	DOCBOOK2PS_COMPILER
	DOCBOOK2RTF_COMPILER
	DOCBOOK2TEX_COMPILER
	DOCBOOK2TEXI_COMPILER
	DOCBOOK2TXT_COMPILER
)
