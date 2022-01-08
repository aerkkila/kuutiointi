all: kuutio

kuutiotied=kuutio.c lue_siirrot.c python_savel.c liity_muistiin.c
kuut_ots=kuutio.h kuution_grafiikka.h python_savel.h
kuut_libs=-lSDL2 -lm

kuutio: liity_muistiin ${kuutiotied} ${kuut_ots} kuution_käyttöliittymä.c kuution_grafiikka.c
	gcc -Wall -o kuutio ${kuutiotied} ${kuut_libs} -Ofast

kuutio0: liity_muistiin ${kuutiotied} ${kuut_ots} kuution_käyttöliittymä.c kuution_grafiikka.c
	gcc -gdwarf-2 -g3 -Wall -o kuutio ${kuutiotied} ${kuut_libs} -O0

autokuutio: liity_muistiin ${kuutiotied} ${kuut_ots} automaattikuutio.c automaattisiirrot.c kuution_grafiikka.c
	gcc -Wall -o autokuutio kuutio.c kuution_kommunikointi.c muistin_jako.c ${kuut_libs} -Ofast -D EI_SAVEL_MAKRO -D AUTOMAATTI

autokuutio0: liity_muistiin ${kuutiotied} ${kuut_ots} automaattikuutio.c automaattisiirrot.c kuution_grafiikka.c
	gcc -gdwarf-2 -g3 -Wall -o autokuutio kuutio.c kuution_kommunikointi.c muistin_jako.c ${kuut_libs} -O0 -D EI_SAVEL_MAKRO -D AUTOMAATTI

laskentakuutio: kuutio.c laskentakuutio.c kuutio.h
	gcc -o laskentakuutio kuutio.c -D__EI_GRAFIIKKAA__ -D__EI_SEKUNTIKELLOA__ -Ofast -pthread
laskentakuutio0: kuutio.c laskentakuutio.c kuutio.h
	gcc -o laskentakuutio -gdwarf-2 -g3 -Wall kuutio.c -D__EI_GRAFIIKKAA__ -D__EI_SEKUNTIKELLOA__ -O0 -pthread -DDEBUG

liity_muistiin: ../liity_muistiin.[ch]
	ln -sf ../liity_muistiin.c .
	ln -sf ../liity_muistiin.h .
