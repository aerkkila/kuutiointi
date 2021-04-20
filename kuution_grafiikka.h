#include "kuutio.h"

#ifndef __KUUTION_GRAFIIKKA__
#define __KUUTION_GRAFIIKKA__

inline koordf __attribute((always_inline)) puorauta(koordf xyz, koordf kulmat) {
  /*x-pyöräytys*/
  float x = xyz.a[0], y = xyz.a[1], z = xyz.a[2];
  float x1,y1,z1;
  y1 = y*cosf(kulmat.a[0]) - z*sinf(kulmat.a[0]);
  z1 = y*sinf(kulmat.a[0]) + z*cosf(kulmat.a[0]);
  y = y1; z = z1;
  /*y-pyöräytys*/
  x1 = x*cosf(kulmat.a[1]) + z*sinf(kulmat.a[1]);
  z1 = -x*sinf(kulmat.a[1]) + z*cosf(kulmat.a[1]);
  x = x1; z = z1;
  /*z-pyöräytys*/
  x1 = x*cosf(kulmat.a[2]) - y*sinf(kulmat.a[2]);
  y1 = x*sinf(kulmat.a[2]) + y*cosf(kulmat.a[2]);
  x = x1; y = y1;
  return (koordf){{x,y,z}};
}
inline void __attribute((always_inline)) aseta_vari(vari v) {
  SDL_SetRenderDrawColor(kuva.rend, v.v[0], v.v[1], v.v[2], 255);
}

#define RUUTU(tahko, i, j) (((tahko)*kuutio.N*kuutio.N + (i)*kuutio.N + (j))*4)
#define RUUTUINT3(A) RUUTU(A.a[0], A.a[1], A.a[2])

#endif

void tee_ruutujen_koordtit();
koordf ruudun_nurkka(int tahko, int iRuutu, int jRuutu, int nurkkaInd);
void piirra_suunnikas(void* koordf2tai3, int onko2vai3);
koordf2* jarjestaKoord2(koordf2* ret, koordf2* ktit, int akseli, int pit);
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit);
void piirra_kuvaksi();
void piirra_viiva(void* karg1, void* karg2, int onko2vai3, int paksuus);
void korosta_tahko(int tahko);
void korosta_ruutu(void* ktit, int onko2vai3);
void korosta_siivu(int tahko, int kaista);
void kaantoanimaatio(int tahko, int kaista, koordf akseli, double maara, double aika);
