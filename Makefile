all: | skello kuutio kellonajat

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c liity_muistiin.c asetelma.c listat.c lomituslajittelu.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h liity_muistiin.h
libs=-lSDL2 -lSDL2_ttf -lm

skello: ${tiedostot} ${otsakkeet}
	gcc -gdwarf-2 -g3 -o skello ${tiedostot} ${libs} -Og

kuutio:
	cd kuutio.d && make

kellonajat: kellonajat.c listat.c
	gcc -g -Wall -shared -o kellonajat.so -fPIC kellonajat.c listat.c -lm -Ofast

kellonajat0: kellonajat.c listat.c
	gcc -g -Wall -shared -o kellonajat.so -fPIC kellonajat.c listat.c -lm -O0
