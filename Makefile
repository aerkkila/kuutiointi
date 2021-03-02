all: kajastin

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c ääni.c
incdir=-I/usr/include/SDL2 -I/home/antterkk/c
libdir=-L/home/antterkk/c
libs=-lSDL2 -lSDL2_ttf -llistat -llista_math -lm

kajastin: kuutio *.[ch]
	gcc -gdwarf-2 -g3 -Wall -o kajastin ${tiedostot} ${incdir} ${libdir} ${libs}

kuutio: kuutio.[ch]
	gcc -gdwarf-2 -g3 -Wall -o kuutio kuutio.c ${incdir} ${libdir} ${libs} -O3
