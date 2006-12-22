inherit subversion

IUSE="gps"

ESVN_REPO_URI="http://svn.berlios.de/svnroot/repos/s3d/trunk"
ESVN_BOOTSTRAP="./autogen.sh --no-configure"

DESCRIPTION="a 3d network display server"
HOMEPAGE="http://s3d.berlios.de/"

LICENSE="GPL-2 LGPL"
KEYWORDS="~x86 ~amd64"
SLOT="0"

DEPEND="${RDEPEND}
	>=sys-devel/autoconf-2.59
	>=sys-devel/automake-1.9
	sys-devel/flex
	>=sys-devel/libtool-1.5
	>=dev-db/sqlite-3
	"
RDEPEND="!media-gfx/s3d
	>=media-libs/libsdl-1.2.7
	>=media-libs/freetype-2
	>=dev-libs/glib-2
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

	gps? ( sci-geosciences/gpsd )	

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

