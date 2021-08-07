#ifndef __listat_h__
#define __listat_h__

#include <stdlib.h>

typedef struct {
  int pit;
  int patka;
  int tilaa;
  int sij;
  size_t koko;
  char** taul;
} slista;

typedef struct {
  int pit;
  int patka;
  int tilaa;
  int sij;
  size_t koko;
  int* taul;
} ilista;

typedef struct {
  int pit;
  int patka;
  int tilaa;
  int sij;
  size_t koko;
  float* taul;
} flista;

typedef struct {
  int pit;
  int patka;
  int tilaa;
  int sij;
  size_t koko;
  void* taul;
} lista;

#define NYT_OLEVA(lis) ((lis)->taul+(lis)->sij)
#define VIIMEINEN(lis) ((lis)->taul+(lis)->pit-1)
#define FOR_LISTA(lis) for((lis)->sij=0; (lis)->sij<(lis)->pit; (lis)->sij++)

#define alusta_lista(a, b) _alusta_lista(a, sizeof(b))
void* _alusta_lista(int, size_t);
void jatka_listaa(void*, int);
void tuhoa_slista(slista**);
void tuhoa_lista(void*);

void slistalle_kopioiden(slista* restrict, const char* restrict);
void flistalle(flista* restrict, float);
void ilistalle(ilista* restrict, int);
void poista_slistalta_viimeinen(slista* restrict);
void poista_slistalta(slista* restrict, int);
void poista_listalta(void*, int);
void rajaa_lista(void*, int alku, int loppu);
void tuhjenna_slista(slista* restrict);
slista* slistaksi(const char* restrict s, const char* restrict erotin);
void slista_sprintf(char* kohde, const char* restrict muoto, slista* restrict lis);
void* monista_listan_taulukko(const void* lv);
#endif
