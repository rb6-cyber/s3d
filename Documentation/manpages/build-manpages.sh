#! /bin/sh -e

for i in 1 3 9; do
	for man in man${i}/*.sgml; do
		if [ -e ${man}  ] ; then
			docbook-to-man ${man} >${man%.sgml}.${i};
		fi
	done
done
