#ifndef __kuutio__
#define __kuutio__

typedef enum {
  _U = 0,
  _F,
  _R,
  _L,
  _D,
  _B
} sivu_e;

typedef struct {char v[3];} vari;

#define VARI(r,g,b) ((vari){{r,g,b}})

typedef struct {
  char** sivut;
  float rotX; //0:ssa katse on suoraan edestä
  float rotY;
  vari* varit;
  char sivuja;
  char N; //NxNxN-kuutio
  char nakuvat;
  SDL_Window* ikkuna;
  SDL_Renderer* rend;
} kuutio_t;

  
#endif
