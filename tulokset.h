#ifndef __TULOKSET__
#define __TULOKSET__

#include <listat.h>
#include "rakenteet.h"

avgtulos avgn(flista* l, int n, int pois);
strlista* tee_tiedot(strlista* tiedot, flista* fl, int* avgind);
strlista* tee_lisatiedot(tkset_s* t, strlista* sektus, int alkuind, int n);
int hae_paikka(float f, flista* l);
int hae_silistalta(strlista* l, int i);
int poista_jarjlistalta(int i, tkset_s *t);
void numerointi_miinus_miinus(strlista*, int);
void lisaa_listoille(tkset_s* t, char* kello, time_t hetki);
void poista_listoilta(tkset_s*, int);
float lue_kellosta(char* s);
//struct tm aikaero(time_t t1, time_t t2);
char tallenna(tkset_s* t, char* tiednimi);
char lue_tiedosto(kaikki_s* k, char* tiednimi);
void tee_jarjlista(tkset_s* t);
char* float_kelloksi(char* kello, float f);
void muuta_sakko(tkset_s* t, char* teksti, int ind);
sakko_e hae_sakko(char*);

#endif
