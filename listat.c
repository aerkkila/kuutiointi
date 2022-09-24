#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "listat.h"

void* _alusta_lista(int p, size_t k) {
  lista* l = calloc(1, sizeof(lista));
  l->patka = p;
  l->tilaa = p;
  l->koko = k;
  l->taul = malloc(p*k);
  return l;
}

void jatka_listaa(void* lv, int n) {
  lista* l = lv;
  while(l->pit+n > l->tilaa)
    l->taul = realloc(l->taul, (l->tilaa+=l->patka)*l->koko);
  l->pit += n;
}

void tuhoa_slista(slista** sl) {
  slista *tmp = *sl;
  FOR_LISTA(tmp)
    free(*NYT_OLEVA(tmp));
  free(tmp->taul);
  free(tmp);
  *sl = NULL;
}

void tuhoa_lista(void* lv) {
  lista* l = *(lista**)lv;
  free(l->taul);
  free(l);
  *(lista**)lv = NULL;
}

void slistalle_kopioiden(slista* restrict sl, const char* restrict str) {
  jatka_listaa(sl, 1);
  *VIIMEINEN(sl) = strdup(str);
}

void slistalle(slista* restrict sl, char* str) {
  jatka_listaa(sl, 1);
  *VIIMEINEN(sl) = str;
}

void flistalle(flista* restrict fl, float lisattava) {
  jatka_listaa(fl, 1);
  *VIIMEINEN(fl) = lisattava;
}
void ilistalle(ilista* restrict fl, int lisattava) {
  jatka_listaa(fl, 1);
  *VIIMEINEN(fl) = lisattava;
}

void poista_slistalta_viimeinen(slista* restrict sl) {
  free(sl->taul[sl->pit-1]);
  sl->pit--;
}

/*vaihdetaan osoittimet eikä dataa,
  joten vapautetaan poistettava eikä suinkaan viimeistä*/
void poista_slistalta(slista* restrict sl, int id) {
  free(sl->taul[id]);
  for(; id<sl->pit-1; id++)
    sl->taul[id] = sl->taul[id+1];
  sl->pit--;
}

void poista_listalta(void* lv, int id) {
  lista* ll = lv;
  const size_t k = ll->koko;
  id *= k;
  for(; id<(ll->pit-1)*k; id+=k)
    memcpy(ll->taul+id, ll->taul+id+k, k);
  ll->pit--;
}

void rajaa_lista(void* lv, int alku, int loppu) {
  lista* ll = lv;
  if(loppu <= alku)
    return;
  int n = loppu-alku;
  int patkia = n/ll->patka + 1;
  void* uusi = malloc(ll->patka*patkia*ll->koko);
  memcpy(uusi, ll->taul+alku*ll->koko, n*ll->koko);
  free(ll->taul);
  ll->pit = n;
  ll->tilaa = patkia*ll->patka;
  ll->taul = uusi;
}

void rajaa_slista(slista* sl, int alku, int loppu) {
  if(loppu <= alku)
    return;
  int n = loppu-alku;
  int patkia = n/sl->patka + 1;
  char** uusi = malloc(sl->patka*patkia*sl->koko);
  memcpy(uusi, sl->taul+alku, n*sl->koko);
  for(int i=0; i<alku; i++)
    free(sl->taul[i]);
  for(int i=loppu; i<sl->pit; i++)
    free(sl->taul[i]);
  free(sl->taul);
  sl->taul = uusi;
  sl->pit = n;
  sl->tilaa = patkia*sl->patka;
}

void tuhjenna_slista(slista* restrict sl) {
  FOR_LISTA(sl)
    free(*NYT_OLEVA(sl));
  sl->pit=0;
  if(sl->tilaa != sl->patka) {
    sl->taul = realloc(sl->taul, sl->patka*sizeof(char*));
    sl->tilaa = sl->patka;
  }
}

void slista_sprintf(char* kohde, const char* restrict muoto, slista* restrict lis) {
  FOR_LISTA(lis) {
    sprintf(kohde, muoto, *NYT_OLEVA(lis));
    kohde += strlen(kohde);
  }
}

slista* slistaksi(const char* restrict s, const char* restrict erotin) {
  char* str = strdup(s);
  char* str0 = str;
  slista* l = alusta_lista(10, char*);
  char* ptr;
  while( (ptr = strstr(str, erotin)) ) {
    *ptr = '\0';
    slistalle_kopioiden(l, str);
    str += strlen(str) + strlen(erotin);
  }
  /*viimeinen ei pääty erottimeen*/
  slistalle_kopioiden(l,str);
  free(str0);
  return l;
}
