# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

macro(PkgStatusPartial_Later _errmsg)
	set(pgk_statuspartial "${pgk_status}\n\t${_errmsg}")
endmacro(PkgStatusPartial_Later _errmsg)

macro(Collected_PkgStatusPartial _modulename)
	if (pgk_statuspartial)
		message(STATUS "${_modulename} will be built without some features because of missing packages: ${pgk_statuspartial}\n\t"
			"The exact names can differ depending on the distribution.")
	endif (pgk_statuspartial)
endmacro(Collected_PkgStatusPartial _modulename)
