pkgname=skello
pkgver=202101.1
pkgrel=1
pkgdesc="A timer and statistics for rubik's cube with an attached NxNxN virtual cube"
arch=('x86_64')
license=('GPL')
depends=('sdl2' 'sdl2_ttf')
makedepends=('sed')
# options=()
source=("$pkgname-$pkgver.tar.gz")
sha256sums=('beba3457cc82754b755950f02b67f49812dc5add486a3420e89f027250805e24')

build() {
	cd "$srcdir"
	make
}

package() {
	cd "$srcdir"
	make DESTDIR=$pkgdir install
}
