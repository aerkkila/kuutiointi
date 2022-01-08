#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#ifndef __EI_SEKUNTIKELLOA__
#include "liity_muistiin.h"
#endif
#include "kuutio.h"

extern kuutio_t kuutio;
extern kuva_t kuva;
extern int viimeViesti;
void lue_siirrot(shmRak_s*);

inline int __attribute__((always_inline)) kaistaksi(char c) {
  int kaista = 1;
  if('a' <= c && c <= 'z')
    kaista = kuutio.N/2;
  return kaista;
}

/*isoilla kuutioilla kaista merkitään numerolla*/
inline int __attribute__((always_inline)) kaistaksi_iso(char c) {
  return c - 0x30;
}

/*Jos kuution koko saavuttaa 20, kaista voi olla suurempi kuin yksinumeroinen luku
  Tämä lukee kaistan ja kasvattaa indeksiä sopivan määrän*/
int kaistaksi_valtava(char* str, int* id) {
  int kaista;
  if(sscanf(str + *id, "%i", &kaista) != 1)
    kaista = 1;
  do
    ++*id;
  while('0' <= str[*id] && str[*id] <= '9');
  return kaista;
}

inline int __attribute__((always_inline)) puoleksi(char c) {
  if('a' <= c && c <= 'z')
    c -= 'a' - 'A';
  switch(c) {
  case 'R':
    return _r;
  case 'L':
    return _l;
  case 'U':
    return _u;
  case 'D':
    return _d;
  case 'B':
    return _b;
  case 'F':
    return _f;
  default:
    return -1;
  }
}

inline int __attribute__((always_inline)) maaraksi(char c) {
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

/*Ajanottosovellus laittaa sekoituksen jaettuun muistiin siirto kerrallaan.
  Tämä funktio lukee siirrot jaetusta muistista ja kääntää kuutiota niitten mukaan*/
void lue_siirrot(shmRak_s* ipc) {
  int puoli=0,kaista=0,maara=0;
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
  if(kuutio.N < 6)
    while(ipc->data[i]) {
      kaista = kaistaksi(ipc->data[i]);
      puoli  = puoleksi(ipc->data[i++]);
      maara  = maaraksi(ipc->data[i++]);
      for(int j=1; j<=kaista; j++)
	siirto(&kuutio, puoli, j, maara);
      while(ipc->data[i] == ' ')
	i++;
    }
  else if(kuutio.N < 20)
    while(ipc->data[i]) {
      kaista = kaistaksi_iso(ipc->data[i++]);
      puoli  = puoleksi(ipc->data[i++]);
      maara  = maaraksi(ipc->data[i++]);
      for(int j=1; j<=kaista; j++)
	siirto(&kuutio, puoli, j, maara);
      while(ipc->data[i] == ' ')
	i++;
    }
  else
    while(ipc->data[i]) {
      kaista = kaistaksi_valtava(ipc->data, &i);
      puoli  = puoleksi(ipc->data[i++]);
      maara  = maaraksi(ipc->data[i++]);
      for(int j=1; j<=kaista; j++)
	siirto(&kuutio, puoli, j, maara);
      while(ipc->data[i] == ' ')
	i++;
    }
  kuva.paivita = 1;
}
