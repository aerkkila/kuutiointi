#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "muistin_jako.h"
#include "kuution_kommunikointi.h"
#include "kuutio.h"

extern kuutio_t kuutio;
extern kuva_t kuva;
extern int viimeViesti;

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

void lue_siirrot(shmRak_s* ipc) {
  int puoli=0;
  char maara=0;
  float aikaraja = 1.0;
  float nukkumaaika = 0.01e6; //µs
  float aika = 0;
  ipc->viesti = ipcAnna_sekoitus;
  viimeViesti = ipcAnna_sekoitus;
  /*ipc->viesti asetetaan nollaksi, kun valmista*/
  while(ipc->viesti && aika < aikaraja) {
    usleep(nukkumaaika);
    aika += nukkumaaika;
  }
  if(ipc->viesti) {
    ipc->viesti = 0;
    printf("Ei luettu siirtoja\n");
    return;
  }
  int i=0;
  while(ipc->data[i]) {
    puoli = puoleksi(ipc->data[i++]);
    maara = maaraksi(ipc->data[i++]);
    siirto(puoli, 0, maara);
    while(ipc->data[i] == ' ')
      i++;
  }
  kuva.paivita = 1;
}
