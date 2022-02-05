#ifndef __tulokset_h__
#define __tulokset_h__

#include "listat.h"

#define VAIHDA(a, b, tyyppi) do {		\
    tyyppi makroapu1234 = a;			\
    a = b;					\
    b = makroapu1234;				\
  } while(0);

typedef enum {
  ei,
  plus,
  dnf
} sakko_etype;
extern sakko_etype sakko;

slista* tee_tiedot(int* avgind);
int* eri_sekunnit(const flista* restrict ftul);
void tee_lisatiedot(char** sektus, int alkuind, int n);
void lisaa_listoille(const char* restrict kello, time_t hetki);
void poista_listoilta(int n);
void poista_listoilta_viimeinen();
float lue_kellosta(const char* restrict s);
char* float_kelloksi(char* kello, float f);
int lue_tiedosto(const char* tiednimi, char* rajaus);
void muuta_sakko(char* teksti, int ind);
int tallenna(const char* restrict tiednimi);

#endif
