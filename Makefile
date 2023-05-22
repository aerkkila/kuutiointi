all: skello kuutio.d/kuutio

tiedostot=skello.c grafiikka.c tulokset.c asetelma1.c listat.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h muistin_jako.h
libs=-lSDL2 -lSDL2_ttf -lm `pkg-config --libs gsl`

skello: ${tiedostot} ${otsakkeet}
	gcc -Wall -Wno-restrict -Og -g -o skello ${tiedostot} ${libs}

kuutio.d/kuutio: kuutio.d/*.[ch]
	cd kuutio.d && make

asetelma1.c: asetelma.c configure.sh
	env KANSIO=/usr/share/skello ./configure.sh $@

install: skello kuutio.d/kuutio äänireuna
	cp -f skello /usr/bin
	cp -f kuutio.d/kuutio /usr/bin/skello_kuutio
	mkdir -p /usr/share/skello
	cp -f kuvaaja.py kuutio.d/savel.py /usr/share/skello
	chmod 755 /usr/share/skello/*.py

uninstall:
	rm -rf /usr/share/skello
	rm -f /usr/bin/skello /usr/bin/skello_kuutio
