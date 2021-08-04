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

#define luo_lista(a, b) _luo_lista(a, sizeof(b))
lista* _luo_lista(int, size_t);
void jatka_listaa(lista*, int);
void tuhoa_slista(slista**);
void tuhoa_lista(lista**);

void slistalle_kopioiden(slista* restrict, const char* restrict);
void poista_slistalta_viimeinen(slista* restrict);
void poista_slistalta(slista* restrict, int);
void poista_listalta(lista* restrict, int);
void tuhjenna_slista(slista* restrict);
#endif
