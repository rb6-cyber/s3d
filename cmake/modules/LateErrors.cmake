# Defines the following macros:
#
# PkgError_Later(errmsg)
# Collected_PkgErrors()

macro(PkgError_Later _errmsg)
	set(pgk_error "${pgk_error}\n\t${_errmsg}")
endmacro(PkgError_Later _errmsg)

macro(Collected_PkgErrors)
	if (pgk_error)
		message(FATAL_ERROR "Following required packages could not be found: ${pgk_error}\n"
			"The exact names can differ depending on the distribution.")
	endif (pgk_error)
endmacro(Collected_PkgErrors)
