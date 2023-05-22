CC = gcc
CFLAGS = -Wall -Ofast

kellonajat.so: kellonajat.c ../listat.c ../listat.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ kellonajat.c ../listat.c -lm
