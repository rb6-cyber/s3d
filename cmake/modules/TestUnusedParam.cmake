# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

include(CheckCSourceCompiles)
check_c_source_compiles("int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) { return 0; }" UNUSEDPARAM_ATTRIBUTE)
check_c_source_compiles("int main(int, char *[]) { return 0; }" UNUSEDPARAM_OMIT)
