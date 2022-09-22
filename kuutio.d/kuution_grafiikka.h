#ifndef __KUUTION_GRAFIIKKA__
#define __KUUTION_GRAFIIKKA__
#include "kuutio.h"

typedef struct {
  SDL_Window* ikkuna;
  SDL_Renderer* rend;
  vari varit[6];
  koordf kannat[3];
  koordf* ruudut;
  int xRes;
  int yRes;
  int sij0; //nurkan paikka katsottaessa suoraan edestä
  float resKuut;
  float mustaOsuus;
  char paivita;
  int korostus;
  int3 ruutuKorostus;
  vari korostusVari;
} kuva_t;

#define VARI(r,g,b) ((vari){{r,g,b}})
#define HAE_RUUTUint3(N,A) hae_ruutu(N, (A).a[0], (A).a[1], (A).a[2])
#define RUUTU(tahko, i, j) (((tahko)*kuutio.N2 + (j)*kuutio.N + (i))*4)
#define RUUTUINT3(A) RUUTU(A.a[0], A.a[1], A.a[2])
#define VARIINT3(rtu) (kuva.varit[(int)kuutio.sivut[SIVUINT3(kuutio.N,rtu)]])
#define RISTITULO(v1,v2,tyyppi) ((tyyppi){{v1.a[1]*v2.a[2] - v1.a[2]*v2.a[1], v1.a[2]*v2.a[0] - v1.a[0]*v2.a[2], v1.a[0]*v2.a[1] - v2.a[0]*v1.a[1]}})

extern kuva_t kuva;
extern SDL_Texture* alusta[];

koordf yleispyöräytys(koordf koord, koordf aks, float kulma);
koordf pyöräytä(koordf xyz, koordf kulmat);
kuva_t* suora_sivu_kuvaksi(int puoli);
int mikä_tahko(int x, int y);
int piste_alueella(float x, float y, int n, ...);
void tee_ruutujen_koordtit();
koordf ruudun_nurkka(int tahko, int iRuutu, int jRuutu, int nurkkaInd);
void piirrä_suunnikas(koordf*);
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit);
void piirrä_kuvaksi();
void piirrä_viiva(void* karg1, void* karg2, int onko2vai3, int paksuus);
int korosta_tahko(int tahko);
void korosta_ruutu(void* ktit, int onko2vai3);
void korosta_siivu(int3 siivu);
void kääntöanimaatio(int tahko, int kaista, koordf akseli, double maara, double aika);
double hetkiNyt();

inline void __attribute__((always_inline)) aseta_vari(vari v) {
  SDL_SetRenderDrawColor(kuva.rend, v.v[0], v.v[1], v.v[2], 255);
}

inline float __attribute__((always_inline)) ristitulo_z(koordf a, koordf b) {
  return a.a[0]*b.a[1] - a.a[1]*b.a[0];
}

inline koordf __attribute__((always_inline)) suuntavektori(koordf* p0, koordf* p1) {
  return (koordf){{p1->a[0]-p0->a[0], p1->a[1]-p0->a[1], p1->a[2]-p0->a[2]}};
}

#endif
