# Copyright 2004-2008 S3D Team
# Distributed under the terms of the GNU General Public License v2

inherit eutils

DESCRIPTION="library for loading 3D models of many file types"
HOMEPAGE="https://gna.org/projects/libg3d/"
SRC_URI="http://download.gna.org/${PN}/${P}.tar.gz"

LICENSE="GPL-2 LGPL-2.1"
SLOT="0"
KEYWORDS="x86"
IUSE="gtk"

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
	epatch "${FILESDIR}"/${PN}-0.0.3-link_math.patch

	econf \
		$(use_enable gtk gtktest) \
		--prefix=/usr/ \
		|| die "econf failed"

	emake || die "emake failed"
}

src_install() {
	emake DESTDIR="${D}" install || die
}
