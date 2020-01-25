# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

file(GLOB man1 "${Documentation_SOURCE_DIR}/manpages/man1/*.1")
install(FILES ${man1} DESTINATION "${MAN_INSTALL_DIR}/man1")
file(GLOB man3 "${Documentation_SOURCE_DIR}/manpages/man3/*.3")
install(FILES ${man3} DESTINATION "${MAN_INSTALL_DIR}/man3")
file(GLOB man9 "${Documentation_SOURCE_DIR}/manpages/man9/*.9")
install(FILES ${man9} DESTINATION "${MAN_INSTALL_DIR}/man9")
