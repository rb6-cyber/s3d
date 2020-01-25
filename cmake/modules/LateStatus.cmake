# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

macro(PkgStatus_Later _errmsg)
	set(pgk_status "${pgk_status}\n\t${_errmsg}")
endmacro(PkgStatus_Later _errmsg)

macro(Collected_PkgStatus _modulename)
	if (pgk_status)
		message(STATUS "${_modulename} will not be build because of missing packages: ${pgk_status}\n\t"
			"The exact names can differ depending on the distribution.")
	endif (pgk_status)
endmacro(Collected_PkgStatus _modulename)
