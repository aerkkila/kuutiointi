kaikki: kuutio

kuutiotied=main.c kuutio.c kuution_käyttöliittymä.c kuution_grafiikka.c python_savel.c
kuut_ots=kuutio.h kuution_grafiikka.h ../muistin_jako.h
kuut_libs=-lSDL2 -lm

kuutio: ${kuut_ots} ${kuutiotied}
	gcc -Wall -o $@ main.c ${kuut_libs} -Ofast

autokuutio: ${kuutiotied} ${kuut_ots} automaattikuutio.c
	gcc -Wall -o $@ main.c ${kuut_libs} -Ofast -DEI_SAVEL_MAKRO -DAUTOMAATTI

laskentakuutio: kuutio.[ch] laskentakuutio.c
	gcc -Wall -o $@ laskentakuutio.c -Ofast -pthread

laskentakuutio0: kuutio.c laskentakuutio.c kuutio.h
	gcc -Wall -o $@ laskentakuutio.c -O0 -gdwarf-2 -g3 -pthread -DDEBUG
