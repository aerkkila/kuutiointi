#include <stdio.h>
#include <time.h>
#include "listat.h"
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <math.h>

/*Tätä kutsutaan python-ohjelmasta kellonajat.py kuvaajan piirtämiseksi.
  Palauttaa taulukon montako tulosta on saatu kullakin vuorokauden minuutilla*/

#define KERROIN 0.39894228 // 1/sqrt(2*pi)
#define SIGMA (gausspit*0.3333333333333)
#define GAUSSPAINO(t) ( KERROIN/SIGMA * exp(-0.5*(t)*(t)/(SIGMA*SIGMA)) )

float* gausskertoimet;
static int* toistot0;
int* toistot;
float* palaute;

float* suodata(int);
float* kellonajat(const char*);
void vapauta();

void alusta(const char* tiedosto) {
  FILE *f = fopen(tiedosto, "r");
  if(!f) {
    fprintf(stderr, "Ei tiedostoa \"%s\"\n", tiedosto);
    return;
  }
  ilista* ajat = alusta_lista(512, int);
  gausskertoimet = malloc(1440*sizeof(float));
  toistot0 = calloc(2880,sizeof(int)); //1440*2
  palaute = malloc(1440*sizeof(float));
  setlocale(LC_ALL, "fi_FI.utf8");
  while(1) {
    int tulos, valiluku;
    tulos = fscanf(f, "%*f%i", &valiluku);
    if(tulos == 1) {
      time_t aika = valiluku;
      jatka_listaa(ajat, 1);
      struct tm *paikal = localtime(&aika);
      ajat->taul[ajat->pit-1] = paikal->tm_hour*60 + paikal->tm_min;
      continue;
    }
    if(tulos == EOF)
      break;
    if(tulos == 0 && fscanf(f, "%*s") == EOF)
      break;
  }
  toistot = toistot0+720; //1440/2
  for(int i=0; i<ajat->pit; i++)
    toistot[ajat->taul[i]]++;
  tuhoa_lista(&ajat);
  /*alkuun ja loppuun laitetaan ympärimenot suodatusta varten*/
  for(int i=0; i<720; i++) { //1440/2
    toistot0[i] = toistot0[i+1440];
    toistot[i+1440] = toistot[i];
  }
}

void vapauta() {
  free(toistot0);
  free(gausskertoimet);
  free(palaute);
}

float* suodata(int gausspit) {
  float* gausskert = gausskertoimet + 720; //1440/2
  for(int t=0; t<=gausspit; t++) {
    gausskert[t] = GAUSSPAINO(t);
    gausskert[-t] = GAUSSPAINO(t);
  }
  for(int i=0; i<1440; i++) {
    float summa = 0;
    for(int T=-gausspit; T<=gausspit; T++)
      summa += toistot[i+T]*gausskert[T];
    palaute[i] = summa;
  }
  return palaute;
}
