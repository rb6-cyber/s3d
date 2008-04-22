# Copyright 2004-2008 S3D Team
# Distributed under the terms of the GNU General Public License v2

DESCRIPTION="a 3d network display server"
HOMEPAGE="http://s3d.berlios.de/"
SRC_URI="mirror://berlios/${PN}/${P}.tar.bz2"

LICENSE="GPL-2 LGPL-2.1"
SLOT="0"
KEYWORDS="x86"
IUSE=""

DEPEND="${RDEPEND}
	>=sys-devel/autoconf-2.59
	>=sys-devel/automake-1.9
	sys-devel/flex
	>=sys-devel/libtool-1.5
	"
RDEPEND="!media-gfx/s3d-svn
	!media-libs/libg3d
	>=media-libs/libsdl-1.2.7
	>=media-libs/freetype-2
	>=dev-libs/glib-2
	media-fonts/ttf-bitstream-vera
	media-libs/fontconfig
	media-libs/mesa

	|| ( 	(
			sys-libs/zlib
			x11-libs/libX11
			x11-libs/libXext
			x11-libs/libXi
			x11-libs/libXmu
			x11-libs/libXt
			x11-libs/libXtst )
		virtual/x11 )

	virtual/glut
	"

src_compile() {
	econf \
		--prefix=/usr/ \
		|| die "econf failed"

	emake || die "emake failed"
}

src_install() {
	emake DESTDIR="${D}" install || die
}
