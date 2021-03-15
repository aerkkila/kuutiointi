#ifndef __TULOKSET__
#define __TULOKSET__

#include <listat.h>
#include "cfg.h"

avgtulos avgn(flista* l, int n, int pois);
int* eri_sekunnit(flista* jarj, int* ia, int iapit);
strlista* tee_tiedot(strlista* tiedot, tkset_s* tkset, int* avgind);
strlista* tee_lisatiedot(tkset_s* t, strlista* sektus, int alkuind, int n);
int hae_paikka(float f, flista* l);
int hae_silistalta(strlista* l, int i);
int poista_jarjlistalta(int i, tkset_s *t);
void numerointi_miinus_miinus(strlista*, int);
void lisaa_listoille(tkset_s* t, char* kello, time_t hetki);
void poista_listoilta(tkset_s*, int);
float lue_kellosta(char* s);
char tallenna(tkset_s* t, char* tiednimi);
char lue_tiedosto(char* tiednimi);
void tee_jarjlista(tkset_s* t);
char* float_kelloksi(char* kello, float f);
void muuta_sakko(tkset_s* t, char* teksti, int ind);
sakko_e hae_sakko(char*);

#endif
