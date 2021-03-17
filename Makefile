all: kajastin kuutio

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c muistin_jako.c cfg.c
kuutiotied=kuutio.c kuution_kommunikointi.c muistin_jako.c python_savel.c
incdir=-I/usr/include/SDL2 -I/home/antterkk/c
libdir=-L/home/antterkk/c
libs=-lSDL2 -lSDL2_ttf -llistat -llista_math -lm

kajastin: ${tiedostot} cfg.h
	gcc -gdwarf-2 -g3 -Wall -o kajastin ${tiedostot} ${incdir} ${libdir} ${libs}

kuutio: ${kuutiotied}
	gcc -gdwarf-2 -g3 -Wall -o kuutio ${kuutiotied} ${incdir} ${libdir} ${libs} -O3

kuutio0: ${kuutiotied}
	gcc -gdwarf-2 -g3 -Wall -o kuutio ${kuutiotied} ${incdir} ${libdir} ${libs} -O0
