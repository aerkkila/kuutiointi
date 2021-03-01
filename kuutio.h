#ifndef __kuutio__
#define __kuutio__

typedef enum {
  _u = 0,
  _f,
  _r,
  _l,
  _d,
  _b
} sivu_e;

typedef struct{ char v[3]; } vari;

#define VARI(r,g,b) ((vari){{r,g,b}})

typedef struct {
  char** sivut;
  float rotX; //0:ssa katse on suoraan edestä
  float rotY;
  vari* varit;
  char sivuja;
  char N; //NxNxN-kuutio
  char nakuvat;
} kuutio_t;

typedef struct {
  SDL_Window* ikkuna;
  SDL_Renderer* rend;
  char* pohja;
  int xRes;
  int yRes;
  char paivita;
} kuva_t;

#endif
