all: skello kuutio.d/kuutio kellonajat.so

tiedostot=skello.c grafiikka.c tulokset.c asetelma1.c listat.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h muistin_jako.h
libs=-lSDL2 -lSDL2_ttf -lm

skello: ${tiedostot} ${otsakkeet}
	${CC} ${CFLAGS} -g -o skello ${tiedostot} ${libs}

kuutio.d/kuutio: kuutio.d
	cd kuutio.d && make

kellonajat.so: kellonajat.c listat.c listat.h
	${CC} ${CFLAGS} -Wall -shared -o $@ -fPIC kellonajat.c listat.c -lm

asetelma1.c: asetelma.c configure.sh
	env KANSIO=/usr/share/skello sh configure.sh $@

install: ${tiedostot} ${otsakkeet} kellonajat.so
	mkdir -p ${DESTDIR}/usr/bin
	cp -f skello ${DESTDIR}/usr/bin
	cp -f kellonajat.py ${DESTDIR}/usr/bin/skellonajat
	chmod 755 ${DESTDIR}/usr/bin/skellonajat
	cp -f kuutio.d/kuutio ${DESTDIR}/usr/bin/skello_kuutio
	mkdir -p ${DESTDIR}/usr/share/skello
	cp -f *.bmp *.so kuvaaja.py kuutio.d/savel.py ${DESTDIR}/usr/share/skello
	chmod 755 ${DESTDIR}/usr/share/skello/*.py

uninstall:
	rm -rf /usr/share/skello
	rm -f /usr/bin/skello /usr/bin/skello_kuutio /usr/bin/skellonajat
