#ifndef __TULOKSET__
#define __TULOKSET__

#include <listat.h>
#include "rakenteet.h"

avgtulos avgn(flista* l, int n, int pois);
strlista* tee_tiedot(strlista* tiedot, flista* fl, int* avgind);
strlista* tee_lisatiedot(tkset_s* t, strlista* sektus, int alkuind, int n);
int hae_paikka(float f, flista* l);
int hae_silistalta(strlista* l, int i);
void poista_jarjlistalta(int i, strlista** si, strlista** s, flista** fl);
void lisaa_listoille(tkset_s* t, char* kello, time_t hetki, int* aikoja);
void poista_listoilta(tkset_s*, int);
float lue_kellosta(char* s);

#endif
