include ../config.mk

kaikki: kuutio

kuutiotied=main.c kuutio.c kuution_käyttöliittymä.c kuution_grafiikka.c python_savel.c lue_siirrot.c
kuut_ots=kuutio.h kuution_grafiikka.h ../muistin_jako.h
kuut_libs=-lSDL2 -lm

kuutio: ${kuut_ots} ${kuutiotied}
	$(CC) $(CFLAGS) -o $@ main.c ${kuut_libs} -Ofast

laskentakuutio: kuutio.[ch] laskentakuutio.c
	$(CC) -Wall -o $@ laskentakuutio.c -Ofast -pthread

laskentakuutio0: kuutio.c laskentakuutio.c kuutio.h
	$(CC) -Wall -o $@ laskentakuutio.c -O0 -gdwarf-2 -g3 -pthread -DDEBUG

sarjakuutio: sarjakuutio.c kuutio.c kuution_käyttöliittymä.c kuution_grafiikka.c lue_siirrot.c
	$(CC) -Wall -o $@ $< -g -D__EI_SEKUNTIKELLOA__ -lSDL2 -lm

kuvaaja.out: kuvaaja.cc
	g++ -Wall kuvaaja.cc `pkg-config --cflags --libs Qt5Widgets` -lqcustomplot -o $@ -g

install:
	cp kuutio $(bindir)/skello_kuutio

uninstall:
	rm $(bindir)/skello_kuutio

clean:
	rm -f kuutio kuvaaja.out laskentakuutio laskentakuutio0 autokuutio
