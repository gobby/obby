# Copyright 1999-2004 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Header: /home/cvsroot/gentoo-x86/dev-util/darcs/darcs-0.9.17.ebuild,v 1.2 2004/03/18 08:27:31 kosmikus Exp $

DESCRIPTION="Library for collaborative text editing"
HOMEPAGE="http://darcs.0x539.de/libobby"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ~ppc ~amd64"
IUSE="notest howl"
EDARCS_REPOSITORY="http://darcs.0x539.de/libobby"
EDARCS_GET_CMD="get --verbose"

DEPEND="net-libs/net6
        >=dev-libs/libsigc++-2.0
		howl? ( >=net-misc/howl-0.9.8 )"

RDEPEND=""

inherit darcs

src_compile() {
	local myconf
	sh ./autogen.sh
	
	use notest && myconf="${myconf} --disable-tests"
	use howl && myconf="${myconf} --with-howl"
	
	econf ${myconf} || die "./configure failed"
	emake || die "make failed"
}

src_install() {
	make DESTDIR=${D} install || die
}
