# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2007-2015  Simon Wunderlich <sw@simonwunderlich.de>

PYTHON_MOD=_s3d.so
PERL_MOD=s3d.so
MODS=$(PYTHON_MOD) $(PERL_MOD)

default:	all
all:		$(MODS)
python:		$(PYTHON_MOD)
perl:		$(PERL_MOD)
$(PYTHON_MOD):	s3d.i
	 swig -python s3d.i
	 gcc -fPIC -c s3d_wrap.c `python-config --cflags`
	 gcc -shared s3d_wrap.o -ls3d -o $(PYTHON_MOD) `python-config --ldflags`
$(PERL_MOD):	s3d.i
	swig -perl5 s3d.i
	gcc -fPIC -c s3d_wrap.c  `perl -MExtUtils::Embed -e ccopts`
	ld -G  s3d_wrap.o -lperl -ls3d -o $(PERL_MOD)
clean:
	rm -f $(MODS) *.o s3d_wrap.c s3d.py s3d.pyc s3d.pm
