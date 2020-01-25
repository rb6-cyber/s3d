# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

include(CheckFunctionExists)
check_function_exists(shmget HAVE_SHM)
