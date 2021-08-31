#ifndef __KUUTION_GRAFIIKKA__
#define __KUUTION_GRAFIIKKA__
#include "kuutio.h"

inline koordf __attribute((always_inline)) puorauta(koordf xyz, koordf kulmat) {
  float x = xyz.a[0], y = xyz.a[1], z = xyz.a[2];
  float x1,y1,z1;
  /*x-pyöräytys*/
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
#define k(i) (koord.a[i])
#define u(i) (aks.a[i])
#define co (cosf(kulma))
#define si (sinf(kulma))
inline koordf __attribute__((always_inline)) yleispuorautus(koordf koord, koordf aks, float kulma) {
  float x,y,z;
  x = (k(0) * (co + u(0)*u(0)*(1-co)) +		\
       k(1) * (u(0)*u(1)*(1-co) - u(2)*si) +	\
       k(2) * (u(0)*u(2)*(1-co) + u(1)*si));
  
  y = (k(0) * (u(1)*u(0)*(1-co) + u(2)*si) +	\
       k(1) * (co + u(1)*u(1)*(1-co)) +		\
       k(2) * (u(1)*u(2)*(1-co) - u(0)*si));

  z = (k(0) * (u(2)*u(0)*(1-co) - u(1)*si) + \
       k(1) * (u(2)*u(1)*(1-co) + u(0)*si) + \
       k(2) * (co + u(2)*u(2)*(1-co)));
  return (koordf){{x,y,z}};
}
#undef k
#undef u
#undef co
#undef si
inline void __attribute((always_inline)) aseta_vari(vari v) {
  SDL_SetRenderDrawColor(kuva.rend, v.v[0], v.v[1], v.v[2], 255);
}

void tee_ruutujen_koordtit();
koordf ruudun_nurkka(int tahko, int iRuutu, int jRuutu, int nurkkaInd);
void piirra_suunnikas(koordf*);
koordf2* jarjestaKoord2(koordf2* ret, koordf2* ktit, int akseli, int pit);
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit);
void piirra_kuvaksi();
void piirra_viiva(void* karg1, void* karg2, int onko2vai3, int paksuus);
int korosta_tahko(int tahko);
void korosta_ruutu(void* ktit, int onko2vai3);
void korosta_siivu(int3 siivu);
void kaantoanimaatio(int tahko, int kaista, koordf akseli, double maara, double aika);

#endif
