PYTHON_MOD=_s3d.so
MODS=$(PYTHON_MOD)

default:	all
all:		$(MODS)
$(PYTHON_MOD):	s3d.i
	 swig -python s3d.i
	 gcc -fPIC -c s3d_wrap.c -I/usr/include/python2.3/
	 gcc -shared s3d_wrap.o -ls3d -o $(PYTHON_MOD)
clean:
	rm -f $(MODS) *.o *.py s3d_wrap.c
