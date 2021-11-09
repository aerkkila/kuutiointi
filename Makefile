all: skello kuutio

tiedostot=main.c grafiikka.c käyttöliittymä.c tulokset.c muistin_jako.c asetelma.c listat.c lomituslajittelu.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h
libs=-lSDL2 -lSDL2_ttf -lm
kuutiotied=kuutio.c kuution_grafiikka.c kuution_kommunikointi.c muistin_jako.c python_savel.c
kuut_ots=kuutio.h kuution_grafiikka.h kuution_kommunikointi.h muistin_jako.h python_savel.h
kuut_libs=-lSDL2 -lm

skello: ${tiedostot} ${otsakkeet}
	gcc -gdwarf-2 -g3 -o skello ${tiedostot} ${libs} -Og

kuutio: ${kuutiotied} ${kuut_ots}
	gcc -gdwarf-2 -g3 -Wall -o kuutio ${kuutiotied} ${kuut_libs} -O3

kuutio0: ${kuutiotied} ${kuut_ots}
	gcc -gdwarf-2 -g3 -Wall -o kuutio ${kuutiotied} ${kuut_libs} -O0

autokuutio: ${kuutiotied} ${kuut_ots} automaattikuutio.c automaattisiirrot.c
	gcc -Wall -o autokuutio kuutio.c kuution_grafiikka.c kuution_kommunikointi.c muistin_jako.c ${kuut_libs} -Ofast -D EI_SAVEL_MAKRO -D AUTOMAATTI

kellonajat: kellonajat.c listat.c
	gcc -g -Wall -shared -o kellonajat.so -fPIC kellonajat.c listat.c -lm -O3

kellonajat0: kellonajat.c listat.c
	gcc -g -Wall -shared -o kellonajat.so -fPIC kellonajat.c listat.c -lm -O0
