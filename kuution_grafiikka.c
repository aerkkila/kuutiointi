#include <SDL.h>
#include <stdarg.h>
#include <math.h>
#include "kuutio.h"
#include "kuution_grafiikka.h"

int minKoordInd(koordf *ktit, int akseli, int pit);
int minKoordInd2(koordf2 *ktit, int akseli, int pit);
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit);
koordf2* jarjestaKoord2(koordf2* ret, koordf2* ktit, int akseli, int pit);

#define VAIHDA(a,b,tyyppi) {			\
    tyyppi apu = a;				\
    a = b;					\
    b = apu;					\
  }

#define TEE_RUUTU kuutio.ruudut[RUUTU(tahko,i,j)+nurkka] = ruudun_nurkka(tahko, i, j, nurkka);
void tee_ruutujen_koordtit() {
  for(int tahko=0; tahko<6; tahko++)
    for(int i=0; i<kuutio.N; i++)
      for(int j=0; j<kuutio.N; j++)
	for(int nurkka=0; nurkka<4; nurkka++)
	  /*ruudut jakavat nurkkia keskenään, joten optimoidaan
	    vähentämällä saman laskemista monta kertaa*/
	  if(i>0) {
	    if(nurkka == 0)
	      kuutio.ruudut[RUUTU(tahko,i,j)] = kuutio.ruudut[RUUTU(tahko,i-1,j)+1];
	    else if(nurkka == 3)
	      kuutio.ruudut[RUUTU(tahko,i,j)+3] = kuutio.ruudut[RUUTU(tahko,i-1,j)+2];
	    else
	      TEE_RUUTU;
	  } else
	    TEE_RUUTU;
}
#undef TEE_RUUTU

void piirra_kuvaksi(int tahko) {
  koordf2* ktit = malloc(4*sizeof(koordf2));
  for(int i=0; i<kuutio.N; i++)
    for(int j=0; j<kuutio.N; j++) {
      vari vari = kuutio.varit[(int)kuutio.sivut[tahko][i*kuutio.N+j]];
      SDL_SetRenderDrawColor(kuva.rend, vari.v[0], vari.v[1], vari.v[2], 255);
      piirra_suunnikas(kuutio.ruudut+RUUTU(tahko, i, j), 3);
    }
  free(ktit);
}

koordf ruudun_nurkka(int tahko, int iRuutu, int jRuutu, int nurkkaInd) {
  koordf nurkka, nurkka0; //nurkka0 on nurkan sijainti ennen pyöritystä
  float res = kuva.resKuut/2;
  float i,j;
  /*haetaan oikea nurkka ruudusta (vasen ylä, vasen ala yms)*/
  switch(nurkkaInd) {
  case 0:
    i = 0; j = 0; break;
  case 1:
    i = 1; j = 0; break;
  case 2:
    i = 1; j = 1; break;
  case 3:
    i = 0; j = 1; break;
  default:
    fprintf(stderr, "Virhe (tee_nurkan_koordtit): nurkkaInd = %i", nurkkaInd);
    return (koordf){{NAN, NAN, NAN}};
  }
  /*siirretään oikeaan ruutuun*/
  float resPala = kuva.resKuut/kuutio.N;
  i = resPala * (iRuutu + i);
  j = resPala * (jRuutu + j);
  
  /*i ja j ovat siirrot sivun vasemmasta ylänurkasta, nyt huomioidaan sivu*/
  switch(tahko) {
  case _u:
    nurkka0 = (koordf){{-res+i, res, -res+j}}; break;
  case _d:
    nurkka0 = (koordf){{-res+i, -res, res-j}}; break;
  case _f:
    nurkka0 = (koordf){{-res+i, res-j, res}}; break;
  case _b:
    nurkka0 = (koordf){{res-i, res-j, -res}}; break;
  case _r:
    nurkka0 = (koordf){{res, res-j, res-i}}; break;
  case _l:
    nurkka0 = (koordf){{-res, res-j, -res+i}}; break;
  default:
    fprintf(stderr, "Virhe (tee_nurkan_koordtit): tahko = %i", tahko);
    return (koordf){{NAN, NAN, NAN}};
  }

  float x,y,z;
  /*x-pyöräytys*/
  x = nurkka0.a[0];
  y = nurkka0.a[1]*cosf(kuutio.rotX) - nurkka0.a[2]*sinf(kuutio.rotX);
  z = nurkka0.a[1]*sinf(kuutio.rotX) + nurkka0.a[2]*cosf(kuutio.rotX);
  /*y-pyöräytys*/
  nurkka.a[0] = x*cosf(kuutio.rotY) + z*sinf(kuutio.rotY) + res + kuva.sij0;
  nurkka.a[1] = y - res - kuva.sij0;
  nurkka.a[2] = -x*sinf(kuutio.rotY) + z*cosf(kuutio.rotY);
  return nurkka;
}

void piirra_suunnikas(void* k23, int onko2vai3) {
  koordf2* ktit;
  if(onko2vai3==2) {
    ktit = k23;
  } else if (onko2vai3==3) {
    ktit = malloc(4*sizeof(koordf2));
    for(int i=0; i<4; i++)
      for(int j=0; j<2; j++)
	ktit[i].a[j] = ((koordf*)k23)[i].a[j];
  } else {
    fprintf(stderr, "Virhe: piirra_suunnikas kutsuttiin luvulla %i\n", onko2vai3);
    return;
  }
  koordf2* xnurkat = jarjestaKoord2(NULL, ktit, 0,  4);
  koordf2* ynurkat = jarjestaKoord2(ktit, ktit, 1,  4);

  if(fabs(xnurkat[0].a[0] - xnurkat[1].a[0]) < 0.5) {
    for(int i=xnurkat[0].a[0]; i<xnurkat[3].a[0]; i++)
      for(int j=-ynurkat[3].a[1]; j<-ynurkat[0].a[1]; j++)
	SDL_RenderDrawPoint(kuva.rend, i, j);
    goto VALMIS;
  }
  
  /*valitaan että ymin ja ymax ovat välinurkat, näin ei välttämättä ole, jos on vaakasuoria viivoja*/
  if(ynurkat[0].a[0] == xnurkat[0].a[0] || ynurkat[0].a[0] == xnurkat[3].a[0])
    VAIHDA(ynurkat[0], ynurkat[1], koordf2);
  if(ynurkat[3].a[0] == xnurkat[0].a[0] || ynurkat[3].a[0] == xnurkat[3].a[0])
    VAIHDA(ynurkat[3], ynurkat[2], koordf2);
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
	SDL_RenderDrawPoint(kuva.rend, i, j);
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
  if(onko2vai3 != 2)
    free(ktit);
  free(xnurkat);
}

int minKoordInd(koordf *ktit, int akseli, int pit) {
  int minInd = 0;
  float min = ktit[0].a[akseli];
  for(int i=1; i<pit; i++)
    if(ktit[i].a[akseli] < min) {
      min = ktit[i].a[akseli];
      minInd = i;
    }
  return minInd;
}

int minKoordInd2(koordf2 *ktit, int akseli, int pit) {
  int minInd = 0;
  float min = ktit[0].a[akseli];
  for(int i=1; i<pit; i++)
    if(ktit[i].a[akseli] < min) {
      min = ktit[i].a[akseli];
      minInd = i;
    }
  return minInd;
}

koordf2* jarjestaKoord2(koordf2* ret, koordf2* ktit, int akseli, int pit) {
  if(!ret)
    ret = malloc(pit*sizeof(koordf));
  koordf2* juoksu = ret;
  koordf2 apu;
  for(int i=0; i<pit; i++)
    ret[i] = ktit[i];
  for(int i=0; i<pit; i++) {
    int ind = minKoordInd2(juoksu, akseli, pit-i);
    apu = *juoksu;
    *juoksu = juoksu[ind];
    juoksu[ind] = apu;
    juoksu++;
  }
  return ret;
}

/*luo uuden, jos ret == NULL*/
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit) {
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

void korosta_tahko(int tahko) {
  int paksuus = 10;
  SDL_SetRenderDrawColor(kuva.rend, 80, 200, 140, 255);
  piirra_viiva(kuutio.ruudut+RUUTU(tahko, 0, 0), kuutio.ruudut+RUUTU(tahko, kuutio.N-1, 0)+1, 3, paksuus);
  kuva.paivita=1;
}

/*jos k2 == NULL, k1, on taulukko, jossa on molemmat*/
void piirra_viiva(void* karg1, void* karg2, int onko2vai3, int paksuus) {
  koordf2 k1, k2;
  if(onko2vai3 == 2) {
    k1 = *(koordf2*)karg1;
    if(karg2)
      k2 = *(koordf2*)karg2;
    else
      k2 = (((koordf2*)karg1))[1];
  } else {
    k1 = (koordf2){{((koordf*)karg1)[0].a[0], ((koordf*)karg1)[0].a[1]}};
    if(karg2)
      k2 = (koordf2){{((koordf*)karg2)[0].a[0], ((koordf*)karg2)[0].a[1]}};
    else
      k2 = (koordf2){{((koordf*)karg1)[1].a[0], ((koordf*)karg1)[1].a[1]}};
  }
  SDL_RenderDrawLine(kuva.rend, k1.a[0], -k1.a[1], k2.a[0], -k2.a[1]);
}
