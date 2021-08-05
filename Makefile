all: kajastin kuutio

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c muistin_jako.c asetelma.c listat.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h
libs=-lSDL2 -lSDL2_ttf -lm
kuutiotied=kuutio.c kuution_grafiikka.c kuution_kommunikointi.c muistin_jako.c python_savel.c
kuut_ots=kuutio.h kuution_grafikka.h kuution_kommunikointi.h muistin_jako.h python_savel.h
kuut_libs=-lSDL2 -lm

kajastin: ${tiedostot} ${otsakkeet}
	gcc -gdwarf-2 -g3 -Wall -o kajastin ${tiedostot} ${libs}

kuutio: ${kuutiotied} ${kuut_ots}
	gcc -gdwarf-2 -g3 -Wall -o kuutio ${kuutiotied} ${kuut_libs} -O3

kuutio0: ${kuutiotied} ${kuut_ots}
	gcc -gdwarf-2 -g3 -Wall -o kuutio ${kuutiotied} ${kuut_libs} -O0
