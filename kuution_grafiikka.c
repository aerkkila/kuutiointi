#include <SDL.h>
#include <stdarg.h>
#include <math.h>
#include "kuutio.h"

void tee_nurkan_koordtit() {
  float x,y,z;
  float res2 = kuva->res1/2;
  float xsij0 = -res2, ysij0 = res2, zsij0 = res2;
  /*x-pyöräytys*/
  x = xsij0;
  y = ysij0*cosf(kuutio->rotX) - zsij0*sinf(kuutio->rotX);
  z = ysij0*sinf(kuutio->rotX) + zsij0*cosf(kuutio->rotX);
  /*y-pyöräytys*/
  kuva->nurkka.a[0] = x*cosf(kuutio->rotY) + z*sinf(kuutio->rotY) - xsij0 + kuva->sij0;
  kuva->nurkka.a[1] = y - ysij0 - kuva->sij0;
  kuva->nurkka.a[2] = -x*sinf(kuutio->rotY) + z*cosf(kuutio->rotY) - zsij0;
}

void tee_kantavektorit() {
  float x,y,z;
  float pit = kuva->res1/kuutio->N;
  
  for(int i=0; i<3; i++) {
    if(i==0)      {x = pit; y = 0;    z = 0;}
    else if(i==1) {x = 0;   y = -pit; z = 0;}
    else          {x = 0;   y = 0;    z = -pit;}

    /*x-pyöräytys*/
    x = x;
    y = y*cosf(kuutio->rotX) - z*sinf(kuutio->rotX);
    z = y*sinf(kuutio->rotX) + z*cosf(kuutio->rotX);

    /*y-pyöräytys*/
    kuva->kannat[i].a[0] = x*cosf(kuutio->rotY) + z*sinf(kuutio->rotY);
    kuva->kannat[i].a[1] = y;
    kuva->kannat[i].a[2] = -x*sinf(kuutio->rotY) + z*cosf(kuutio->rotY);
  }
}

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

koordf* ruudun_nurkat(koordf *a, int tahko, char i, char j) {
  switch(tahko) {
  case _u:
    a[0] = vektSum(3, kuva->nurkka,
		   vektKertI(kuva->kannat[2], kuutio->N-j-1),
		   vektKertI(kuva->kannat[0], i));
    break;
  case _f:
    a[0] = vektSum(3, kuva->nurkka,
		   vektKertI(kuva->kannat[0], i),
		   vektKertI(kuva->kannat[1], j));
    break;
  case _r:
    a[0] = vektSum(4, kuva->nurkka,
		   vektKertI(kuva->kannat[0], kuutio->N),
		   vektKertI(kuva->kannat[2], i),
		   vektKertI(kuva->kannat[1], j));
    break;
  case _d:
    a[0] = vektSum(4, kuva->nurkka,
		   vektKertI(kuva->kannat[1], kuutio->N),
		   vektKertI(kuva->kannat[0], i),
		   vektKertI(kuva->kannat[2], j));
    break;
  case _b:
    a[0] = vektSum(4, kuva->nurkka,
		   vektKertI(kuva->kannat[2], kuutio->N),
		   vektKertI(kuva->kannat[0], kuutio->N-i-1),
		   vektKertI(kuva->kannat[1], j));
    break;
  case _l:
    a[0] = vektSum(3, kuva->nurkka,
		   vektKertI(kuva->kannat[2], kuutio->N-i-1),
		   vektKertI(kuva->kannat[1], j));
    break;
  }
  switch(tahko) {
  case _u:
  case _d:
    a[1] = vektSum(2, a[0], kuva->kannat[0]);
    a[2] = vektSum(2, a[1], kuva->kannat[2]);
    a[3] = vektSum(2, a[0], kuva->kannat[2]);
    break;
  case _f:
  case _b:
    a[1] = vektSum(2, a[0], kuva->kannat[0]);
    a[2] = vektSum(2, a[1], kuva->kannat[1]);
    a[3] = vektSum(2, a[0], kuva->kannat[1]);
    break;
  case _r:
  case _l:
    a[1] = vektSum(2, a[0], kuva->kannat[1]);
    a[2] = vektSum(2, a[1], kuva->kannat[2]);
    a[3] = vektSum(2, a[0], kuva->kannat[2]);
    break;
  }
  return a;
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
  
  koordf *nurkat = malloc(4*sizeof(koordf));
  ruudun_nurkat(nurkat, tahko, iRuutu, jRuutu);
  koordf* xnurkat = jarjestaKoord(NULL, nurkat, 0,  4);
  koordf* ynurkat = jarjestaKoord(nurkat, nurkat, 1, 4);
  nurkat = NULL;

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
}
