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
typedef struct{
  short x;
  short y;
  //short z; jätetään pois koska kuva projisoidaan
} koord;

#define VARI(r,g,b) ((vari){{r,g,b}})

typedef struct {
  char** sivut;
  float rotX; //0:ssa katse on suoraan edestä
  float rotY;
  vari* varit;
  char sivuja;
  char N; //NxNxN-kuutio
  char nakuvat; //UFRLDB
} kuutio_t;

typedef struct {
  SDL_Window* ikkuna;
  SDL_Renderer* rend;
  char** pohjat;
  koord** koordtit;
  int xRes;
  int yRes;
  int sij0; //nurkan paikka kun katsotaan suoraan edestä
  int res1;
  int pit;
  char paivita;
} kuva_t;

#endif
