# Maintainer: Anttoni Erkkilä <anttoni.erkkila+aur@protonmail.com>
pkgname=skello
pkgver=202101.1
pkgrel=1
pkgdesc="A timer and statistics for rubik's cube with an attached NxNxN virtual cube"
arch=('x86_64')
license=('GPL')
depends=('sdl2' 'sdl2_TTF')
makedepends=('sed')
# options=()
source=("$pkgname-$pkgver.tar.gz"
        "$pkgname-$pkgver.patch")
md5sums=()
validpgpkeys=()

build() {
	cd "$pkgname-$pkgver"
	make
}

package() {
	cd "$pkgname-$pkgver"
	make install
}
