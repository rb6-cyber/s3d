SUBDIRS = . doc m4 include plugins src tests

pkgconfigdir=${libdir}/pkgconfig

pkgconfig_DATA = libg3d.pc

deb:
	fakeroot dpkg-buildpackage
