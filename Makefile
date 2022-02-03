all: skello kuutio.d/kuutio kellonajat.so äänireuna libäänen_opettaminen.so

tiedostot=skello.c grafiikka.c tulokset.c asetelma1.c listat.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h muistin_jako.h
libs=-lSDL2 -lSDL2_ttf -lm

skello: ${tiedostot} ${otsakkeet}
	gcc -Wall -Wno-restrict -Og -g -o skello ${tiedostot} ${libs}

kuutio.d/kuutio: kuutio.d/*.[ch]
	cd kuutio.d && make

kellonajat.so: kellonajat.c listat.c listat.h
	gcc -Wall -Ofast -shared -o $@ -fPIC kellonajat.c listat.c -lm

asetelma1.c: asetelma.c configure.sh
	env KANSIO=/usr/share/skello ./configure.sh $@

äänireuna: äänireuna.c äänireuna.h
	gcc -Wall -g -o $@ äänireuna.c -lasound -lm -pthread -läänen_opettaminen

libäänen_opettaminen.so: äänen_opettaminen.c
	gcc -Wall -g -shared -fPIC -o $@ äänen_opettaminen.c -lSDL2

install: skello kuutio.d/kuutio kellonajat.so äänireuna libäänen_opettaminen.so
	cp -f skello /usr/bin
	cp -f kellonajat.py /usr/bin/skellonajat
	chmod 755 /usr/bin/skellonajat
	cp -f kuutio.d/kuutio /usr/bin/skello_kuutio
	cp -f äänireuna /usr/bin/
	mkdir -p /usr/share/skello
	cp -f libäänen_opettaminen.so /usr/lib/
	cp -f *.bmp kellonajat.so kuvaaja.py kuutio.d/savel.py /usr/share/skello
	chmod 755 /usr/share/skello/*.py

uninstall:
	rm -rf /usr/share/skello
	rm -f /usr/lib/libäänen_opettaminen.so
	rm -f /usr/bin/skello /usr/bin/skello_kuutio /usr/bin/skellonajat /usr/bin/äänireuna
