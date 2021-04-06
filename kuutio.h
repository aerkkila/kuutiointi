#ifndef __kuutio__
#define __kuutio__
#include <SDL.h>

typedef enum {
  _r,
  _u,
  _f,
  _l,
  _d,
  _b
} sivu_e;

typedef struct{ char v[3]; } vari;
typedef struct{ float a[3]; } koordf;
typedef struct {float a[2];} koordf2;
typedef struct {int a[3];} int3;

#define VARI(r,g,b) ((vari){{r,g,b}})
#define SIGN(i) ((i<0)? -1: (i==0)? 0: 1)
#define ABS(i) ((i<0)? -i: i)
#define VAIHDA(a,b,tyyppi) {			\
    tyyppi apu = a;				\
    a = b;					\
    b = apu;					\
  }

typedef struct {
  char** sivut;
  koordf* ruudut;
  koordf xyz;
  vari* varit;
  char N; //NxNxN-kuutio
  char ratkaistu;
} kuutio_t;

typedef struct {
  SDL_Window* ikkuna;
  SDL_Renderer* rend;
  int xRes;
  int yRes;
  int sij0; //nurkan paikka kun katsotaan suoraan edestä
  float resKuut;
  float mustaOsuus;
  char paivita;
  int korostus;
  int3 ruutuKorostus;
  vari korostusVari;
} kuva_t;

#endif

extern kuutio_t kuutio;
extern kuva_t kuva;
extern int3 akst[6];

void siirto(int puoli, char kaista, char maara);
kuva_t* suora_sivu_kuvaksi(int puoli);
void paivita();
int3 hae_ruutu(int tahko, int i0, int j0);
