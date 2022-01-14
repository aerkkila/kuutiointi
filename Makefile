all: skello kuutio.d kellonajat.so

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c liity_muistiin.c asetelma1.c listat.c lomituslajittelu.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h liity_muistiin.h
libs=-lSDL2 -lSDL2_ttf -lm

skello: ${tiedostot} ${otsakkeet}
	gcc -g -o skello ${tiedostot} ${libs} -O3

kuutio.d: kuutio.d/kuutio
	cd kuutio.d && make

kellonajat.so: kellonajat.c listat.c
	gcc -Wall -shared -o $@ -fPIC kellonajat.c listat.c -lm -Ofast

asetelma1.c: asetelma.c asetelma.sh
	sh asetelma.sh $@

install: ${tiedostot} ${otsakkeet} kellonajat.so
	mkdir -p ${DESTDIR}/bin
	cp -f skello ${DESTDIR}/bin
	cp -f kellonajat.py ${DESTDIR}/bin/skello_kellonajat.py
	cp -f kuutio.d/kuutio ${DESTDIR}/bin/skello_kuutio
	mkdir -p ${DESTDIR}/share/skello
	cp -f *.bmp *.so kuvaaja.py kuutio.d/sävel.py ${DESTDIR}/share/skello

uninstall:
	rm -rf ${DESTDIR}/share/skello
	rm -f ${DESTDIR}/bin/skello ${DESTDIR}/bin/skello_kuutio ${DESTDIR}/bin/skello_kellonajat.py
