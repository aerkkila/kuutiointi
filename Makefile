all: kajastin

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c
incdir=-I/usr/include/SDL2 -I/home/antterkk/c
libdir=-L/home/antterkk/c
libs=-lSDL2 -lSDL2_ttf -llistat -llista_math -lm

kajastin: ${tiedostot}
	gcc -g -Wall -o kajastin ${tiedostot} ${incdir} ${libdir} ${libs} -O0
