pkgname=skello
pkgver=202101.1
pkgrel=1
pkgdesc="A timer and statistics for rubik's cube with an attached NxNxN virtual cube"
arch=('x86_64')
license=('GPL')
depends=('sdl2' 'sdl2_ttf')
makedepends=('sed')
# options=()
#source=("$pkgname-$pkgver.tar.gz")
md5sums=()
validpgpkeys=()

build() {
	cd "$srcdir/$pkgname-$pkgver"
	make
}

package() {
	cd "$srcdir/$pkgname-$pkgver"
	echo $pkgdir
#	make DESTDIR=$pkgdir install
}
