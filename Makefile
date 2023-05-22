include config.mk

all: skello kuutio.d/kuutio

tiedostot=skello.c grafiikka.c tulokset.c asetelma1.c listat.c
otsakkeet=asetelma.h grafiikka.h listat.h tulokset.h muistin_jako.h
libs=-lSDL2 -lSDL2_ttf -lm `pkg-config --libs gsl`

skello: ${tiedostot} ${otsakkeet}
	$(CC) $(CFLAGS) -Wno-restrict -o $@ ${tiedostot} ${libs}

kuutio.d/kuutio: kuutio.d
	$(MAKE) -C kuutio.d

asetelma1.c: asetelma.c configure.sh
	env KANSIO=$(sharedir)/skello ./configure.sh $@

install: skello kuutio.d/kuutio
	cp -f skello $(bindir)
	mkdir -p $(sharedir)/skello
	cp -f kuvaaja.py kuutio.d/savel.py $(sharedir)/skello
	chmod 755 $(sharedir)/skello/*.py
	$(MAKE) -C kuutio.d install

uninstall:
	rm -rf $(sharedir)/skello
	rm -f $(bindir)/skello
	$(MAKE) -C kuutio.d uninstall

clean:
	rm skello
	$(MAKE) -C kuutio.d clean
