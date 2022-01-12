all: skello kuutio.d kellonajat.so

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c liity_muistiin.c asetelma1.c listat.c lomituslajittelu.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h liity_muistiin.h
libs=-lSDL2 -lSDL2_ttf -lm

skello: ${tiedostot} ${otsakkeet}
	gcc -gdwarf-2 -g3 -o skello ${tiedostot} ${libs} -Og

kuutio.d:
	cd kuutio.d && make

kellonajat.so: kellonajat.c listat.c
	gcc -Wall -shared -o $@ -fPIC kellonajat.c listat.c -lm -Ofast

asetelma1.c: asetelma.c asetelma.sh
	sh asetelma.sh $@
