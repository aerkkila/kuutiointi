all: skello kuutio.d/kuutio kellonajat.so

tiedostot=skello.c grafiikka.c tulokset.c asetelma1.c listat.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h muistin_jako.h
libs=-lSDL2 -lSDL2_ttf -lm

skello: ${tiedostot} ${otsakkeet}
	gcc -gdwarf-2 -g3 -o skello ${tiedostot} ${libs} -Og

kuutio.d/kuutio: kuutio.d
	cd kuutio.d && make

kellonajat.so: kellonajat.c listat.c listat.h
	gcc -Wall -shared -o $@ -fPIC kellonajat.c listat.c -lm -Ofast

asetelma1.c: asetelma.c asetelma.sh
	sh asetelma.sh $@
