# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

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
