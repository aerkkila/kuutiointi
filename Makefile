all: kajastin

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c ääni.c
incdir=-I/usr/include/SDL2 -I/home/antterkk/c
libdir=-L/home/antterkk/c
libs=-lSDL2 -lSDL2_ttf -llistat -llista_math -lm

kajastin: kuutio *.[ch]
	gcc -gdwarf-2 -g3 -Wall -o kajastin ${tiedostot} ${incdir} ${libdir} ${libs}

kuutio: kuutio.[ch] kuution_kommunikointi.[ch]
	gcc -gdwarf-2 -g3 -Wall -o kuutio kuutio.c kuution_kommunikointi.c ${incdir} ${libdir} ${libs} -O3

kuutio0: kuutio.[ch] kuution_kommunikointi.[ch]
	gcc -gdwarf-2 -g3 -Wall -o kuutio kuutio.c kuution_kommunikointi.c ${incdir} ${libdir} ${libs} -O0
