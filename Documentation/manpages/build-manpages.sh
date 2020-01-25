#! /bin/sh -e
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>

for i in 1 3 9; do
	for man in man${i}/*.sgml; do
		if [ -e ${man}  ] ; then
			xmlto man ${man} -o `dirname ${man}`
			sed -i 's/^\(\.\\"\s* Generator: DocBook XSL Stylesheets\).*$/\1/' ${man%.sgml}.${i}
			sed -i 's/^\(\.\\"\)\s*Date.*$/\1/' ${man%.sgml}.${i}
			sed -i 's/^\(\.TH "[^"]*" "[^"]*" "\)[^"]*\(".*\)$/\1\2/' ${man%.sgml}.${i}

			# remove trailing spaces
			sed -i 's/^\(.*\)\s\s*$/\1/' ${man%.sgml}.${i}
		fi
	done
done
