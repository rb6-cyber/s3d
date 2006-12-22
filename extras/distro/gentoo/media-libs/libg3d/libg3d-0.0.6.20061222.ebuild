inherit subversion eutils

IUSE="gtk xml"

ESVN_REPO_URI="http://svn.gna.org/svn/libg3d/trunk"
ESVN_OPTIONS="--revision {2006-12-22}"
ESVN_BOOTSTRAP="./autogen.sh"

DESCRIPTION="library for loading 3D models of many file types"
HOMEPAGE="https://gna.org/projects/libg3d/"

LICENSE="GPL-2 LGPL"
KEYWORDS="~x86 ~amd64"
SLOT="0"

DEPEND="${RDEPEND}
	dev-util/gtk-doc
	>=sys-devel/autoconf-2.59
	>=sys-devel/automake-1.9
	sys-devel/flex
	>=sys-devel/libtool-1.5
	"
RDEPEND="!<=media-gfx/s3d-0.1.1
	>=dev-libs/glib-2

	gtk? ( >=x11-libs/gtk+-2 )
	xml? ( dev-libs/libxml2 )
	"

src_unpack() {
	subversion_src_unpack
	cd "${S}"
	epatch ${FILESDIR}/libg3d-0.0.6.20061222-missingflexheaders.patch.bz2
}

src_compile() {
	econf \
		$(use_enable gtk gtktest) \
		$(use_enable xml xmltest) \
		--prefix=/usr/ \
		|| die "econf failed"

	emake || die "emake failed"
}

src_install() {
	emake DESTDIR="${D}" install || die
}

