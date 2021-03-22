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

#define VARI(r,g,b) ((vari){{r,g,b}})

typedef struct {
  char** sivut;
  float rotX; //0:ssa katse on suoraan edestä
  float rotY;
  vari* varit;
  char sivuja; // aina 6
  char N; //NxNxN-kuutio
  char nakuvat;
  char ratkaistu;
} kuutio_t;

typedef struct {
  SDL_Window* ikkuna;
  SDL_Renderer* rend;
  char** pohjat;
  koordf kannat[3]; //kuution kantavektorit, pituus on palan särmän pituus*/
  koordf nurkka; //C-nurkka eli ylä-etuvasen
  int xRes;
  int yRes;
  int sij0; //nurkan paikka kun katsotaan suoraan edestä
  float res1;
  int pit;
  char paivita;
} kuva_t;

#endif

extern kuutio_t* kuutio;
extern kuva_t* kuva;

void siirto(int puoli, char kaista, char maara);
kuva_t* suora_sivu_kuvaksi(int puoli);
