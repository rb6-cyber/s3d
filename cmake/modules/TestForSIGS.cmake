# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

include(CheckFunctionExists)
check_function_exists(signal HAVE_SIGS_FUNC)

include(CheckCSourceCompiles)
check_c_source_compiles("#include<signal.h>
	int main() {
	sig_t test;
	return 0;
}" SIG_T_DEFINED)

if (HAVE_SIGS_FUNC AND SIG_T_DEFINED)
	set(HAVE_SIGS TRUE)
endif (HAVE_SIGS_FUNC AND SIG_T_DEFINED)
