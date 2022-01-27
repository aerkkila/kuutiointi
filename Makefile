all: skello kuutio.d/kuutio kellonajat.so äänireuna

tiedostot=skello.c grafiikka.c tulokset.c asetelma1.c listat.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h muistin_jako.h
libs=-lSDL2 -lSDL2_ttf -lm

skello: ${tiedostot} ${otsakkeet}
	gcc -Wall -Wno-restrict -Og -g -o skello ${tiedostot} ${libs}

kuutio.d/kuutio: kuutio.d
	cd kuutio.d && make

kellonajat.so: kellonajat.c listat.c listat.h
	gcc -Wall -Ofast -shared -o $@ -fPIC kellonajat.c listat.c -lm

asetelma1.c: asetelma.c configure.sh
	env KANSIO=/usr/share/skello ./configure.sh $@

äänireuna: äänireuna.c
	gcc -Wall -g -o äänireuna äänireuna.c -lasound -lm -pthread

install: skello kuutio.d/kuutio kellonajat.so äänireuna
	cp -f skello /usr/bin
	cp -f kellonajat.py /usr/bin/skellonajat
	chmod 755 /usr/bin/skellonajat
	cp -f kuutio.d/kuutio /usr/bin/skello_kuutio
	cp -f äänireuna /usr/bin/äänireuna
	mkdir -p /usr/share/skello
	cp -f *.bmp *.so kuvaaja.py kuutio.d/savel.py /usr/share/skello
	chmod 755 /usr/share/skello/*.py

uninstall:
	rm -rf /usr/share/skello
	rm -f /usr/bin/skello /usr/bin/skello_kuutio /usr/bin/skellonajat /usr/bin/äänireuna
