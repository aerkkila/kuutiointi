#ifndef __kuutio__
#define __kuutio__
#include <SDL.h>

typedef enum {
  _u = 0,
  _f,
  _r,
  _d,
  _b,
  _l
} sivu_e;

typedef struct{ char v[3]; } vari;
typedef struct{ float a[3]; } koordf;
typedef struct {float a[2];} koordf2;

#define VARI(r,g,b) ((vari){{r,g,b}})

typedef struct {
  char** sivut;
  koordf* ruudut;
  float rotX; //0:ssa katse on suoraan edestä
  float rotY;
  float rotZ;
  vari* varit;
  char N; //NxNxN-kuutio
  char nakuvat;
  char ratkaistu;
} kuutio_t;

typedef struct {
  SDL_Window* ikkuna;
  SDL_Renderer* rend;
  int xRes;
  int yRes;
  int sij0; //nurkan paikka kun katsotaan suoraan edestä
  float resKuut;
  char paivita;
} kuva_t;

#endif

extern kuutio_t kuutio;
extern kuva_t kuva;

void siirto(int puoli, char kaista, char maara);
kuva_t* suora_sivu_kuvaksi(int puoli);
