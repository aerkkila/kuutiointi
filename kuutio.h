#ifndef __kuutio__
#define __kuutio__
#include <SDL2/SDL.h>

typedef enum {
  _r,
  _u,
  _f,
  _l,
  _d,
  _b
} sivu_e;

typedef struct{ unsigned char v[3]; } vari;
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
#define HAE_RUUTUint3(A) hae_ruutu((A).a[0], (A).a[1], (A).a[2])
#define RUUTU(tahko, i, j) (((tahko)*kuutio.N*kuutio.N + (i)*kuutio.N + (j))*4)
#define RUUTUINT3(A) RUUTU(A.a[0], A.a[1], A.a[2])

typedef struct {
  char** sivut;
  koordf* ruudut;
  koordf xyz;
  vari* varit;
  int N; //NxNxN-kuutio
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
  int3 siivuKorostus; //akseli, siivuluku, kääntösuunta
  vari korostusVari;
} kuva_t;

inline float __attribute__((always_inline)) ristitulo_z(koordf a, koordf b) {
  return a.a[0]*b.a[1] - a.a[1]*b.a[0];
}

inline koordf __attribute__((always_inline)) suuntavektori(koordf* p0, koordf* p1) {
  return (koordf){{p1->a[0]-p0->a[0], p1->a[1]-p0->a[1], p1->a[2]-p0->a[2]}};
}

#define RISTITULO(v1,v2,tyyppi) (tyyppi){{v1.a[1]*v2.a[2] - v1.a[2]*v2.a[1], v1.a[2]*v2.a[0] - v1.a[0]*v2.a[2], v1.a[0]*v2.a[1] - v2.a[0]*v1.a[1]}}
#endif

extern kuutio_t kuutio;
extern kuva_t kuva;
extern int3 akst[6];
extern SDL_Texture* alusta[];

int mika_tahko(int x, int y);
int piste_alueella(float x, float y, int n, ...);
void siirto(int puoli, int kaista, int maara);
kuva_t* suora_sivu_kuvaksi(int puoli);
void paivita();
int3 hae_ruutu(int tahko, int i0, int j0);
int3 hae_siivu(int3 ruutu);
