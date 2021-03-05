#include <sys/time.h>
#include <stdio.h>
#include "kuution_kommunikointi.h"
#include "kuutio.h"

extern kuutio_t* kuutio;
extern kuva_t* kuva;

inline int __attribute__((always_inline)) puoleksi(char c) {
  if('A' <= c && c <= 'Z')
    c += 'a' - 'A';
  switch(c) {
  case 'r':
    return _r;
  case 'l':
    return _l;
  case 'u':
    return _u;
  case 'd':
    return _d;
  case 'b':
    return _b;
  case 'f':
    return _f;
  default:
    return -1;
  }
}

inline char __attribute__((always_inline)) maaraksi(char c) {
  switch(c) {
  case ' ':
    return 1;
  case '2':
    return 2;
  case '\'':
    return 3;
  default:
    return -1;
  }
}

void lue_siirrot() {
  struct timeval aika0;
  printf("Syötä_siirrot\n");
  gettimeofday(&aika0, NULL);
  int puoli=0;
  char maara=0;
  char luenta[6];
  while(1) {
    if(scanf("%s", luenta) < 1)
      break;
    printf("%s\n", luenta);
    puoli = puoleksi(luenta[0]);
    if(puoli < 0)
      break;
    maara = maaraksi(luenta[1]);
    if(maara < 0)
      break;
    siirto(puoli, 1, maara);
  }
  for(int i=0; i<6; i++)
    suora_sivu_kuvaksi(i);
  kuva->paivita = 1;
}
