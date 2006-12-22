IUSE="gtk"

SRC_URI="http://download.gna.org/${PN}/${P}.tar.gz"

DESCRIPTION="library for loading 3D models of many file types"
HOMEPAGE="https://gna.org/projects/libg3d/"

LICENSE="GPL-2 LGPL"
KEYWORDS="x86"
SLOT="0"

DEPEND="${RDEPEND}
	>=sys-devel/autoconf-2.59
	>=sys-devel/automake-1.9
	sys-devel/flex
	>=sys-devel/libtool-1.5
	"
RDEPEND="!<=media-gfx/s3d-0.1.1
	>=dev-libs/glib-2

	gtk? ( >=x11-libs/gtk+-2 )
	"

src_compile() {
	econf \
		$(use_enable gtk gtktest) \
		--prefix=/usr/ \
		|| die "econf failed"

	emake || die "emake failed"
}

src_install() {
	emake DESTDIR="${D}" install || die
}

