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
#define GAUSSPIT 45
#define SIGMA (GAUSSPIT/3.0)
#define GAUSSPAINO(t) ( KERROIN/SIGMA * exp(-0.5*(t)*(t)/(SIGMA*SIGMA)) )

static float* gausskert;
static float* suodata(int* restrict);
float* kellonajat(const char*);
void vapauta(void*);

float* kellonajat(const char* tiedosto) {
  FILE *f = fopen(tiedosto, "r");
  if(!f) {
    fprintf(stderr, "Ei tiedostoa \"%s\"\n", tiedosto);
    return NULL;
  }
  ilista* ajat = alusta_lista(512, int);
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
  /*alkuun ja loppuun laitetaan ympärimenot suodatusta varten*/
  int *toistot = calloc(1440+2*GAUSSPIT,sizeof(int));
  int *toistot1 = toistot+GAUSSPIT;
  for(int i=0; i<ajat->pit; i++)
    toistot1[ajat->taul[i]]++;
  tuhoa_lista(&ajat);
  /*ympärimenot*/
  for(int i=0; i<GAUSSPIT; i++) {
    toistot[i] = toistot[i+1440];
    toistot1[i+1440] = toistot1[i];
  }
  /*nopeutetaan laskemalla gaussin kertoimet vain kerran*/
  float* gausskert0 = malloc((2*GAUSSPIT+1)*sizeof(float));
  gausskert = gausskert0 + GAUSSPIT;
  for(int t=-GAUSSPIT; t<=GAUSSPIT; t++)
    gausskert[t] = GAUSSPAINO(t);
  
  float* palaute = suodata(toistot1);
  free(toistot);
  free(gausskert0);
  return palaute;
}

void vapauta(void* taul) {
  free(taul);
}

static float* suodata(int* restrict itaul) {
  float* tulos = malloc(1440*sizeof(float));
  for(int i=0; i<1440; i++) {
    float summa = 0;
    for(int T=-GAUSSPIT; T<=GAUSSPIT; T++)
      summa += itaul[i+T]*gausskert[T];
    tulos[i] = summa;
  }
  return tulos;
}
