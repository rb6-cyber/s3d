# Copyright 2004-2008 S3D Team
# Distributed under the terms of the GNU General Public License v2

inherit subversion

DESCRIPTION="a 3d network display server"
HOMEPAGE="http://s3d.berlios.de/"
ESVN_REPO_URI="http://svn.berlios.de/svnroot/repos/s3d/trunk"

LICENSE="GPL-2 LGPL-2.1"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE="gps doc xcomposite"

DEPEND="${RDEPEND}
	sys-devel/flex
	>=dev-db/sqlite-3
	>=dev-util/cmake-2.4.4
	"
RDEPEND="!media-gfx/s3d
	>=media-libs/libsdl-1.2.7
	>=media-libs/freetype-2
	dev-libs/libxml2
	media-fonts/ttf-bitstream-vera
	media-libs/fontconfig
	>=media-libs/libg3d-0.0.6
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

	doc? ( app-text/docbook-sgml-utils )
	gps? ( sci-geosciences/gpsd )
	xcomposite? (
		x11-libs/libXcomposite
		x11-libs/libXdamage
		x11-libs/libXfixes
		x11-libs/libXrender
	)
	"

src_compile() {
	cmake -DCMAKE_INSTALL_PREFIX=/usr/ ${cmake_opts} || die "cmake failed"
	emake || die "emake failed"
}

src_install() {
	emake DESTDIR="${D}" install || die
	dodoc ChangeLog README TODO
}
