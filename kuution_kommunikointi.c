#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "kuution_kommunikointi.h"
#include "kuutio.h"
#include "cfg.h"

const char shmTied[] = "shmTied.txt";
extern kuutio_t* kuutio;
extern kuva_t* kuva;

shmRak_s* liity_muistiin() {
  int avain = ftok(shmTied, PROJ_ID);
  if(avain < 0) {
    perror("Virhe, ftok");
    return NULL;
  }
  int shmid = shmget(avain, SHM_KOKO, IPC_CREAT|IPC_EXCL);
  if(shmid < 0) {
    perror("Virhe, shmget");
    return NULL;
  }
  shmRak_s *r = shmat(shmid, NULL, 0);
  if(r < 0) {
    perror("Virhe, shmat");
    return NULL;
  }
  return r;
}

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
  ipc->viesti = anna_sekoitus;
  /*ipc->viesti asetetaan nollaksi, kun valmista*/
  while(ipc->viesti && aika < aikaraja) {
    usleep(nukkumaaika);
    aika += nukkumaaika;
  }
  if(ipc->viesti) {
    ipc->viesti = 0;
    return;
  }
  int i=0;
  while(ipc->data[i]) {
    puoli = puoleksi(ipc->data[i++]);
    maara = maaraksi(ipc->data[i++]);
    siirto(puoli, 1, maara);
    while(ipc->data[i] == ' ')
      i++;
  }
  for(int i=0; i<6; i++)
    suora_sivu_kuvaksi(i);
  kuva->paivita = 1;
}
