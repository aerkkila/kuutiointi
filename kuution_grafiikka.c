#include <SDL.h>
#include <stdarg.h>
#include <math.h>
#include "kuutio.h"
#include "kuution_grafiikka.h"

inline koordf __attribute__((always_inline)) vektKertI(koordf a, int b) {
  for(int i=0; i<3; i++)
    a.a[i] *= b;
  return a;
}

koordf vektSum(int n, ...) {
  va_list argl;
  va_start(argl, n);
  koordf r = (koordf){{0,0,0}};
  for(int i=0; i<n; i++) {
    koordf a = va_arg(argl, koordf);
    for(int i=0; i<3; i++)
      r.a[i] += a.a[i];
  }
  return r;
}

koordf* tee_ruudun_koordtit(koordf* a, int tahko, char i, char j) {
  if(!a)
    a = malloc(4*sizeof(koordf));
  float x,y,z;
  float res2 = kuva->resKuut/2;
  float resPala = kuva->resKuut/kuutio->N;
  koordf sij0 = (koordf){{-res2, res2, res2}};
  koordf xkanta = (koordf){{resPala, 0, 0}};
  koordf ykanta = (koordf){{0, -resPala, 0}};
  koordf zkanta = (koordf){{0, 0, -resPala}};
  switch(tahko) {
  case _u:
    a[0] = vektSum(3, sij0,
		   vektKertI(zkanta, kuutio->N-j-1),
		   vektKertI(xkanta, i));
    break;
  case _f:
    a[0] = vektSum(3, sij0,
		   vektKertI(xkanta, i),
		   vektKertI(ykanta, j));
    break;
  case _r:
    a[0] = vektSum(4, sij0,
		   vektKertI(xkanta, kuutio->N),
		   vektKertI(zkanta, i),
		   vektKertI(ykanta, j));
    break;
  case _d:
    a[0] = vektSum(4, sij0,
		   vektKertI(ykanta, kuutio->N),
		   vektKertI(xkanta, i),
		   vektKertI(zkanta, j));
    break;
  case _b:
    a[0] = vektSum(4, sij0,
		   vektKertI(zkanta, kuutio->N),
		   vektKertI(xkanta, kuutio->N-i-1),
		   vektKertI(ykanta, j));
    break;
  case _l:
    a[0] = vektSum(3, sij0,
		   vektKertI(zkanta, kuutio->N-i-1),
		   vektKertI(ykanta, j));
    break;
  }
  switch(tahko) {
  case _u:
  case _d:
    a[1] = vektSum(2, a[0], xkanta);
    a[2] = vektSum(2, a[1], zkanta);
    a[3] = vektSum(2, a[0], zkanta);
    break;
  case _f:
  case _b:
    a[1] = vektSum(2, a[0], xkanta);
    a[2] = vektSum(2, a[1], ykanta);
    a[3] = vektSum(2, a[0], ykanta);
    break;
  case _r:
  case _l:
    a[1] = vektSum(2, a[0], ykanta);
    a[2] = vektSum(2, a[1], zkanta);
    a[3] = vektSum(2, a[0], zkanta);
    break;
  }
  for(int i=0; i<4; i++) {
    /*x-pyöräytys*/
    x = a[i].a[0];
    y = a[i].a[1]*cosf(kuutio->rotX) - a[i].a[2]*sinf(kuutio->rotX);
    z = a[i].a[1]*sinf(kuutio->rotX) + a[i].a[2]*cosf(kuutio->rotX);
    /*y-pyöräytys*/
    a[i].a[0] = x*cosf(kuutio->rotY) + z*sinf(kuutio->rotY) - sij0.a[0] + kuva->sij0;
    a[i].a[1] = y - sij0.a[1] - kuva->sij0;
    a[i].a[2] = -x*sinf(kuutio->rotY) + z*cosf(kuutio->rotY) - sij0.a[2];
  }
  return a;
}

void tee_ruutujen_koordtit() {
  for(int tahko=0; tahko<6; tahko++)
    for(int i=0; i<kuutio->N; i++)
      for(int j=0; j<kuutio->N; j++)
	tee_ruudun_koordtit(kuutio->ruudut[RUUTU(tahko,i,j)], tahko, i, j);
}

int minKoordInd(koordf* ktit, int akseli, int pit) {
  int minInd = 0;
  float min = ktit[0].a[akseli];
  for(int i=1; i<pit; i++)
    if(ktit[i].a[akseli] < min) {
      min = ktit[i].a[akseli];
      minInd = i;
    }
  return minInd;
}

/*luo uuden, jos ret == NULL*/
koordf* jarjestaKoord(koordf *ret, koordf* ktit, int akseli, int pit) {
  if(!ret)
    ret = malloc(pit*sizeof(koordf));
  koordf* juoksu = ret;
  koordf apu;
  for(int i=0; i<pit; i++)
    ret[i] = ktit[i];
  for(int i=0; i<pit; i++) {
    int ind = minKoordInd(juoksu, akseli, pit-i);
    apu = *juoksu;
    *juoksu = juoksu[ind];
    juoksu[ind] = apu;
    juoksu++;
  }
  return ret;
}

#define VAIHDA(a,b,tyyppi) {			\
    tyyppi apu = a;				\
    a = b;					\
    b = apu;					\
  }

void piirra_ruutu(int tahko, int iRuutu, int jRuutu) {
  vari vari = kuutio->varit[(int)kuutio->sivut[tahko][iRuutu*kuutio->N+jRuutu]];
  SDL_SetRenderDrawColor(kuva->rend, vari.v[0], vari.v[1], vari.v[2], 255);
  
  koordf* xnurkat = jarjestaKoord(NULL, kuutio->ruudut[RUUTU(tahko,iRuutu,jRuutu)], 0,  4);
  koordf* ynurkat = jarjestaKoord(NULL, kuutio->ruudut[RUUTU(tahko,iRuutu,jRuutu)], 1,  4);

  if(fabs(xnurkat[0].a[0] - xnurkat[1].a[0]) < 0.5) {
    for(int i=xnurkat[0].a[0]; i<xnurkat[3].a[0]; i++)
      for(int j=-ynurkat[3].a[1]; j<-ynurkat[0].a[1]; j++)
	SDL_RenderDrawPoint(kuva->rend, i, j);
    goto VALMIS;
  }
  
  /*valitaan että ymin ja ymax ovat välinurkat, näin ei välttämättä ole, jos on vaakasuoria viivoja*/
  if(ynurkat[0].a[0] == xnurkat[0].a[0] || ynurkat[0].a[0] == xnurkat[3].a[0])
    VAIHDA(ynurkat[0], ynurkat[1], koordf);
  if(ynurkat[3].a[0] == xnurkat[0].a[0] || ynurkat[3].a[0] == xnurkat[3].a[0])
    VAIHDA(ynurkat[3], ynurkat[2], koordf);
  float kulmakerr1 = (ynurkat[3].a[1]-xnurkat[0].a[1]) / (ynurkat[3].a[0]-xnurkat[0].a[0]);
  float kulmakerr2 = (ynurkat[0].a[1]-xnurkat[0].a[1]) / (ynurkat[0].a[0]-xnurkat[0].a[0]);
  float muisti;

  /*ylä- tai alapuolen kulmakerroin vaihtuu,
    kun saavutetaan kys. puolen nurkka*/
  float y1 = -xnurkat[0].a[1];
  float y2 = -xnurkat[0].a[1];
  for(int patka=0; patka<3; patka++) {
    int xraja = (int)(xnurkat[patka+1].a[0]);
    for(int i=xnurkat[patka].a[0]; i<xraja; i++) {
      for(int j=y1; j<y2; j++)
	SDL_RenderDrawPoint(kuva->rend, i, j);
      y1 -= kulmakerr1;
      y2 -= kulmakerr2;
    }
    if(patka==0)
      if(xnurkat[1].a[1] < xnurkat[2].a[1]) {
	muisti = kulmakerr2;
	kulmakerr2 = (ynurkat[0].a[1]-xnurkat[3].a[1]) / (ynurkat[0].a[0]-xnurkat[3].a[0]);
	y2 = -xnurkat[1].a[1];
      } else {
	muisti = kulmakerr1;
	kulmakerr1 = (ynurkat[3].a[1]-xnurkat[3].a[1]) / (ynurkat[3].a[0]-xnurkat[3].a[0]);
	y1 = -xnurkat[1].a[1];
      }
    else
      if(xnurkat[1].a[1] < xnurkat[2].a[1]) {
	kulmakerr1 = muisti; //suunnikkaat ovat kivoja
      } else {
	kulmakerr2 = muisti;
      }
  }
 VALMIS:
  free(xnurkat);
  free(ynurkat);
}

void piirra_kuvaksi(int tahko) {
  for(int i=0; i<kuutio->N; i++)
    for(int j=0; j<kuutio->N; j++)
      piirra_ruutu(tahko, i, j);
  korosta_tahko(tahko);
}

void korosta_tahko(int tahko) {
  int paksuus = 10;
  SDL_SetRenderDrawColor(kuva->rend, 0, 200, 255, 255);
  koordf* ruutu = malloc(4*sizeof(koordf));
  for(int iRuutu=0; iRuutu<kuutio->N; iRuutu++) {
    /*alaosa*/
    jarjestaKoord(ruutu, kuutio->ruudut[RUUTU(tahko,iRuutu,2)], 1, 4);
    jarjestaKoord(ruutu, ruutu, 0, 2);
    float kulmakerr = (ruutu[1].a[1]-ruutu[0].a[1]) / (ruutu[1].a[0]-ruutu[0].a[0]);
    int iEro = ruutu[1].a[0]-ruutu[0].a[0];
    for(int i=0; i<iEro; i++)
      for(int j=-paksuus/2; j<paksuus/2; j++)
	SDL_RenderDrawPoint(kuva->rend, i+ruutu[0].a[0], j-ruutu[0].a[1]-i*kulmakerr);
    /*yläosa*/
    jarjestaKoord(ruutu, kuutio->ruudut[RUUTU(tahko,iRuutu,0)], 1, 4);
    jarjestaKoord(ruutu, ruutu+2, 0, 2);
    iEro = ruutu[1].a[0]-ruutu[0].a[0];
    for(int i=0; i<iEro; i++)
      for(int j=-paksuus/2; j<paksuus/2; j++)
	SDL_RenderDrawPoint(kuva->rend, i+ruutu[0].a[0], j-ruutu[0].a[1]-i*kulmakerr);
  }
  for(int jRuutu=0; jRuutu<kuutio->N; jRuutu++) {
    /*vasen*/
    jarjestaKoord(ruutu, kuutio->ruudut[RUUTU(tahko,0,jRuutu)], 0, 4);
    jarjestaKoord(ruutu, ruutu, 1, 2);
    float kulmakerr = (ruutu[1].a[0]-ruutu[0].a[0]) / (ruutu[1].a[1]-ruutu[0].a[1]);
    int jEro = -ruutu[0].a[1]+ruutu[1].a[1];
    for(int j=0; j<jEro; j++)
      for(int i=-paksuus/2; i<paksuus/2; i++)
	SDL_RenderDrawPoint(kuva->rend, i+ruutu[1].a[0]-j*kulmakerr, j-ruutu[1].a[1]);
    /*oikea*/
    jarjestaKoord(ruutu, kuutio->ruudut[RUUTU(tahko,2,jRuutu)], 0, 4);
    jarjestaKoord(ruutu, ruutu+2, 1, 2);
    kulmakerr = (ruutu[1].a[0]-ruutu[0].a[0]) / (ruutu[1].a[1]-ruutu[0].a[1]);
    jEro = -ruutu[0].a[1]+ruutu[1].a[1];
    for(int j=0; j<jEro; j++)
      for(int i=-paksuus/2; i<paksuus/2; i++)
	SDL_RenderDrawPoint(kuva->rend, i+ruutu[1].a[0]-j*kulmakerr, j-ruutu[1].a[1]);
  }
  free(ruutu);
}
