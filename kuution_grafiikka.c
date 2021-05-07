#include <SDL.h>
#include <stdarg.h>
#include <math.h>
#include <sys/time.h>
#include "kuutio.h"
#include "kuution_grafiikka.h"

int minKoordInd(koordf *ktit, int akseli, int pit);
int minKoordInd2(koordf2 *ktit, int akseli, int pit);
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit);
koordf2* jarjestaKoord2(koordf2* ret, koordf2* ktit, int akseli, int pit);

#define TEE_RUUTU kuutio.ruudut[RUUTU(tahko,i,j)+nurkka] = ruudun_nurkka(tahko, i, j, nurkka);
void tee_ruutujen_koordtit() {
  for(int tahko=0; tahko<6; tahko++)
    for(int i=0; i<kuutio.N; i++)
      for(int j=0; j<kuutio.N; j++)
	for(int nurkka=0; nurkka<4; nurkka++)
	  TEE_RUUTU;
}
#undef TEE_RUUTU

void piirra_kuvaksi() {
  for(int tahko=0; tahko<6; tahko++)
    for(int i=0; i<kuutio.N; i++)
      for(int j=0; j<kuutio.N; j++) {
	vari vari = kuutio.varit[(int)kuutio.sivut[tahko][i*kuutio.N+j]];
	SDL_SetRenderDrawColor(kuva.rend, vari.v[0], vari.v[1], vari.v[2], 255);
#define A(n) (kuutio.ruudut+RUUTU(tahko,i,j)+n)
	if(ristitulo_z(suuntavektori(A(0), A(3)), suuntavektori(A(0), A(1))) > 0)
	  piirra_suunnikas(kuutio.ruudut+RUUTU(tahko, i, j), 3);
#undef A
      }
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
  float resMusta = resPala*kuva.mustaOsuus/2; // x/2, koska jakautuu kahteen palaan
  i = resPala * (iRuutu + i) + pow(-1,i)*resMusta;
  j = resPala * (jRuutu + j) + pow(-1,j)*resMusta;
  
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
  nurkka0 = puorauta(nurkka0, kuutio.xyz);

  nurkka.a[0] = nurkka0.a[0] + res + kuva.sij0;
  nurkka.a[1] = nurkka0.a[1] - res - kuva.sij0;
  nurkka.a[2] = nurkka0.a[2];
  return nurkka;
}

void piirra_suunnikas(void* k23, int onko2vai3) {
  koordf2* ktit;
  if(onko2vai3==2) {
    ktit = malloc(4*sizeof(koordf2));
    for(int i=0; i<4; i++)
      for(int j=0; j<2; j++)
	ktit[i].a[j] = ((koordf2*)k23)[i].a[j];
  } else if (onko2vai3==3) {
    ktit = malloc(4*sizeof(koordf2));
    for(int i=0; i<4; i++)
      for(int j=0; j<2; j++)
	ktit[i].a[j] = ((koordf*)k23)[i].a[j];
  } else {
    fprintf(stderr, "Virhe: piirra_suunnikas kutsuttiin luvulla %i\n", onko2vai3);
    return;
  }
  koordf2* xnurkat = jarjestaKoord2(ktit, ktit, 0,  4);
  int xEro = (xnurkat[1].a[0] - xnurkat[0].a[0]) >= 0.5;

  float *yUla, *yAla;
  float kulmakerr1, kulmakerr2, y1, y2;
  if(xEro) {
    kulmakerr2 = ((xnurkat[2].a[1]-xnurkat[0].a[1]) /	\
		  (xnurkat[2].a[0]-xnurkat[0].a[0]));
    kulmakerr1 = ((xnurkat[1].a[1]-xnurkat[0].a[1]) /	\
		  (xnurkat[1].a[0]-xnurkat[0].a[0]));
    y1 = -xnurkat[0].a[1];
    y2 = -xnurkat[0].a[1];
    if(kulmakerr1 < kulmakerr2) {
      yAla = &y1;
      yUla = &y2;
    } else {
      yAla = &y2;
      yUla = &y1;
    }
  } else { //kaksi samaa kulmakerrointa, katsotaan näistä ylempi
    int lisa0 = xnurkat[1].a[1] > xnurkat[0].a[1];
    int lisa1 = xnurkat[3].a[1] > xnurkat[2].a[1];
    kulmakerr1 = ((xnurkat[2+lisa1].a[1]-xnurkat[0+lisa0].a[1]) /	\
		  (xnurkat[2+lisa1].a[0]-xnurkat[0+lisa0].a[0]));
    kulmakerr2 = kulmakerr1;
    y1 = -xnurkat[lisa0].a[1];
    yUla = &y1;
    y2 = -xnurkat[!lisa0].a[1];
    yAla = &y2;
  }
  float muisti=0;
  /*ylä- tai alapuolen kulmakerroin vaihtuu,
    kun saavutetaan kys. puolen nurkka*/
  for(int patka=(xEro)? 0:2; patka<3; patka++) {
    int xraja = (int)(xnurkat[patka+1].a[0]);
    for(int i=xnurkat[patka*xEro].a[0]; i<xraja; i++) {
      for(int j=*yUla; j<*yAla; j++)
	SDL_RenderDrawPoint(kuva.rend, i, j);
      y1 -= kulmakerr1;
      y2 -= kulmakerr2;
    }
    if(patka==0) {
      muisti = kulmakerr1;
      kulmakerr1 = (xnurkat[1].a[1]-xnurkat[3].a[1]) / (xnurkat[1].a[0]-xnurkat[3].a[0]);
      y1 = -xnurkat[1].a[1];
    } else {
      kulmakerr2 = muisti; //suunnikkaat ovat kivoja
      y2 = -xnurkat[2].a[1];
    }
  }
  free(ktit);
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

int korosta_tahko(int tahko) {
  if(tahko < 0 || tahko > 5)
    return 0;
  int paksuus = 15;
  aseta_vari(kuva.korostusVari);
  piirra_viiva(kuutio.ruudut+RUUTU(tahko, 0, 0),		\
	       kuutio.ruudut+RUUTU(tahko, kuutio.N-1, 0)+1,	\
	       3, paksuus);
  piirra_viiva(kuutio.ruudut+RUUTU(tahko, kuutio.N-1, 0)+1,		\
	       kuutio.ruudut+RUUTU(tahko, kuutio.N-1, kuutio.N-1)+2,	\
	       3, paksuus);
  piirra_viiva(kuutio.ruudut+RUUTU(tahko, 0, kuutio.N-1)+3,		\
	       kuutio.ruudut+RUUTU(tahko, kuutio.N-1, kuutio.N-1)+2,	\
	       3, paksuus);
  piirra_viiva(kuutio.ruudut+RUUTU(tahko, 0, 0),		\
	       kuutio.ruudut+RUUTU(tahko, 0, kuutio.N-1)+3,	\
	       3, paksuus);
  return 1;
}

void korosta_ruutu(void* ktit, int onko2vai3) {
  int paksuus = 10;
  aseta_vari(kuva.korostusVari);
  if(onko2vai3 == 2) {
    koordf2* k = ktit;
    piirra_viiva(k, k+1, onko2vai3, paksuus);
    piirra_viiva(k+1, k+2, onko2vai3, paksuus);
    piirra_viiva(k+2, k+3, onko2vai3, paksuus);
    piirra_viiva(k+3, k, onko2vai3, paksuus);
  } else {
    koordf* k = ktit;
    piirra_viiva(k, k+1, onko2vai3, paksuus);
    piirra_viiva(k+1, k+2, onko2vai3, paksuus);
    piirra_viiva(k+2, k+3, onko2vai3, paksuus);
    piirra_viiva(k+3, k, onko2vai3, paksuus);
  }
}

void korosta_siivu(int tahko, int kaista) {
  if(tahko < 0 || tahko > 5 || kaista >= kuutio.N)
    return;
  const int paksuus = 15;
  aseta_vari(kuva.korostusVari);
  int akseli = 0;
  int suunta = 0;
  for(int i=0; i<3; i++) {
    if(akst[tahko].a[i] % 3)
      continue;
    akseli = i;
    suunta = SIGN(akst[tahko].a[i]);
    break;
  }
  int3 ruutu0 = hae_ruutu(tahko, kuutio.N+kaista, 0);
  int3 ruutu1 = hae_ruutu(tahko, kuutio.N+kaista, kuutio.N-1);
#define K(arg) kuutio.ruudut+RUUTUINT3(arg)
  piirra_viiva(K(ruutu0), K(ruutu1), 3, paksuus);
  ruutu1 = hae_ruutu(tahko, -1-kaista, 0);
  piirra_viiva(K(ruutu0), K(ruutu1), 3, paksuus);
#undef K
}

/*jos k2 == NULL, k1, on taulukko, jossa on molemmat*/
void piirra_viiva(void* karg1, void* karg2, int onko2vai3, int paksuus) {
  int h2 = paksuus/2;
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
  /*kasvava x-koordinaatti*/
  if(k2.a[0] < k1.a[0])
    VAIHDA(k2, k1, koordf2);
  float yEro = k2.a[1] - k1.a[1];
  float xEro = k2.a[0] - k1.a[0];
  if(xEro < 1) {
    int iAlku = k1.a[0], jAlku = k1.a[1];
    int yEroi = yEro;
    if(yEroi > 0)
      for(int j=0; j<yEroi; j++)
	for(int i=-h2; i<=h2; i++)
	  SDL_RenderDrawPoint(kuva.rend, i+iAlku, -(j+jAlku));
    else
      for(int j=0; j>yEroi; j--)
	for(int i=-h2; i<=h2; i++)
	  SDL_RenderDrawPoint(kuva.rend, i+iAlku, -(j+jAlku));
    return;
  }
  if(fabs(yEro) < 1) {
    int jAlku = k1.a[1], iAlku = k1.a[0];
    int xEroi = xEro;
    if(xEroi > 0)
      for(int i=0; i<xEroi; i++)
	for(int j=-h2; j<=h2; j++)
	  SDL_RenderDrawPoint(kuva.rend, iAlku+i, -(jAlku+j));
    else
      for(int i=0; i>xEroi; i--)
	for(int j=-h2; j<=h2; j++)
	  SDL_RenderDrawPoint(kuva.rend, iAlku+i, -(jAlku+j));
    return;
  }
  float kulma = yEro / xEro;
  double kulmaT = -1/kulma;
  if(kulma >= 0) {
    int jNyt, jSeur, xEroi=xEro, jLoppu=k2.a[1];
    float iAlku, jAlku, jAlkuLoppu, jAlkuSeur, jAlkuNyt;
    iAlku = k1.a[0]; jAlku = k1.a[1];
    /*piirretään ohuita viivoja vierekkäin kunnes saavutetaan haluttu paksuus*/
    iAlku -= cos(atan(kulmaT))*h2;
    jAlku -= sin(atan(kulmaT))*h2; //kasvaa
    jAlkuLoppu = jAlku + (k1.a[1]-jAlku)*2; //loppu < alku

    /*ohuitten viivojen silmukka*/
    while(jAlku >= jAlkuLoppu) {
      jAlkuSeur = jAlku + kulmaT; //seuraava on pienempi
      /*jAlku voi muuttua alle 1:n, joten tähän silmukkaan otetaan toinen muuttuja*/
      jAlkuNyt = jAlku;
      /*tätä silmukkaa tarvitaan, jos kulmaT > 1; monta y-alkua samalla x-alulla*/
      while(jAlkuNyt > jAlkuSeur && jAlkuNyt >= jAlkuLoppu) {
	for(int i=0; i<=xEroi; i++) {
	  jNyt = jAlkuNyt + i*kulma;
	  jSeur = jAlkuNyt + (i+1)*kulma;
	  while(jNyt <= jSeur && jNyt <= jLoppu)
	    SDL_RenderDrawPoint(kuva.rend, i+iAlku, -jNyt++);
	}
	jAlkuNyt--; //kulmaT on negatiivinen
      }
      jAlku += kulmaT;
      iAlku++;
    }
    return;
  }
  /*kulma < 0*/
  int jNyt, jSeur, xEroi=xEro, jLoppu=k2.a[1];
  float iAlku, jAlku, jAlkuLoppu, jAlkuSeur, jAlkuNyt;
  iAlku = k1.a[0]; jAlku = k1.a[1];
  /*piirretään ohuita viivoja vierekkäin kunnes saavutetaan haluttu paksuus*/
  iAlku -= cos(atan(kulmaT))*h2;
  jAlku -= sin(atan(kulmaT))*h2; //vähenee
  jAlkuLoppu = jAlku + (k1.a[1]-jAlku)*2; //loppu > alku
  
  while(jAlku <= jAlkuLoppu) {
    jAlkuSeur = jAlku + kulmaT; //seuraava on suurempi
    jAlkuNyt = jAlku;
    while(jAlkuNyt < jAlkuSeur && jAlkuNyt <= jAlkuLoppu) {
      for(int i=0; i<=xEroi; i++) {
	jNyt = jAlkuNyt + i*kulma;
	jSeur = jAlkuNyt + (i+1)*kulma;
	while(jNyt >= jSeur && jNyt >= jLoppu) //kulma on negatiivinen
	  SDL_RenderDrawPoint(kuva.rend, i+iAlku, -jNyt--);
      }
      jAlkuNyt++; //kulmaT on positiivinen
    }
    jAlku += kulmaT;
    iAlku++;
  }
  return;
}

inline koordf __attribute__((always_inline)) yleispuorautus(koordf koord, koordf aks, float kulma) {
#define k(i) (koord.a[i])
#define u(i) (aks.a[i])
#define co (cosf(kulma))
#define si (sinf(kulma))
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

#define PI 3.14159265
void kaantoanimaatio(int tahko, int kaista, koordf akseli, double maara, double aika) {
  if(kaista)
    return;
  int3 paikka;
  float siirto = kuva.resKuut/2 + kuva.sij0;
  double fps = 30.0;
  double kokoKulma = PI/2*maara;
  double kulmaNyt = 0.0;
  struct timeval hetki;
  gettimeofday(&hetki, NULL);
  double alku = hetki.tv_sec + hetki.tv_usec*1.0e-6;
  double loppu = alku + aika;
  double kului = 0;
  akseli = puorauta(akseli, kuutio.xyz);
  
  while(alku+kului/2 < loppu) {
    float askel = (kokoKulma - kulmaNyt) / (fps * (loppu-alku));
    if(fabs(kulmaNyt+askel) > fabs(kokoKulma))
      break;
    kulmaNyt += askel;
    
#define A kuutio.ruudut[RUUTU(paikka.a[0],paikka.a[1],paikka.a[2])+n]
    for(int i=-1; i<kuutio.N+1; i++)
      for(int j=-1; j<kuutio.N+1; j++) {
	paikka = hae_ruutu(tahko,i,j);
	if(paikka.a[0] < 0)
	  continue;
	for(int n=0; n<4; n++) {
	  A = (koordf){{A.a[0]-siirto, A.a[1]+siirto, A.a[2]}};
	  A = yleispuorautus(A, akseli, askel);
	  A = (koordf){{A.a[0]+siirto, A.a[1]-siirto, A.a[2]}};
	}
      }
    paivita();
    /*pysähdys*/
    gettimeofday(&hetki, NULL);
    double nyt = hetki.tv_sec + hetki.tv_usec*1.0e-6;
    kului = nyt - alku;
    if(kului < 1/fps)
      SDL_Delay((unsigned)((1/fps - kului)*1000));
    
    /*näyttämiseen kuluva aika lasketaan uuden kuvan tekemiseen*/
    gettimeofday(&hetki, NULL);
    alku = hetki.tv_sec + hetki.tv_usec*1.0e-6;
    SDL_RenderPresent(kuva.rend);
  }
  /*viimeinen askel menee loppuun*/
  float askel = kokoKulma-kulmaNyt;
  for(int i=-1; i<kuutio.N+1; i++)
    for(int j=-1; j<kuutio.N+1; j++) {
      paikka = hae_ruutu(tahko,i,j);
      if(paikka.a[0] < 0)
	continue;
      for(int n=0; n<4; n++) {
	A = (koordf){{A.a[0]-siirto, A.a[1]+siirto, A.a[2]}};
	A = yleispuorautus(A, akseli, askel);
	A = (koordf){{A.a[0]+siirto, A.a[1]-siirto, A.a[2]}};
      }
    }
  paivita();
}
#undef A
