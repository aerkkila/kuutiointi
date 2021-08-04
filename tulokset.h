#ifndef __TULOKSET__
#define __TULOKSET__

#include <listat.h>

typedef struct {
  float nyt;
  float min;
  int minind;
  float max;
  int maxind;
} avgtulos;

typedef enum {
  ei = 0,
  plus,
  dnf
} sakko_e;

#endif

extern strlista* strtulos;
extern flista* ftulos;
extern ilista* tuloshetki; //unix-aika tuloksen saamishetkellä
extern strlista* sijarj;
extern flista* fjarj;
extern strlista* strjarj;

avgtulos avgn(flista* l, int n, int pois);
int* eri_sekunnit(flista* jarj, int* ia, int iapit);
strlista* tee_tiedot(strlista* tiedot, int* avgind);
strlista* tee_lisatiedot(strlista* sektus, int alkuind, int n);
int hae_paikka(float f, flista* l);
int hae_silistalta(strlista* l, int i);
int poista_jarjlistalta(int i);
void numerointi_miinus_miinus(strlista*, int);
void lisaa_listoille(char* kello, time_t hetki);
void poista_listoilta_viimeinen();
void poista_listoilta(int);
float lue_kellosta(char* s);
char tallenna(char* tiednimi);
char lue_tiedosto(char* tiednimi, char* rajaus);
void tee_jarjlista();
char* float_kelloksi(char* kello, float f);
void muuta_sakko(char* teksti, int ind);
sakko_e hae_sakko(char*);
