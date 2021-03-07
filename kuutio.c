#include <stdio.h>
#include <SDL.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "kuutio.h"
#include "muistin_jako.h"
#include "kuution_kommunikointi.h"

#define PI 3.14159265358979

/*näkyvät sivut määritellään tavuna, jossa bitit oikealta alkaen ovat
  ylä, etu, oikea, ala, taka, vasen;
  esim (0x01 | 0x02) | 0x04 tarkoittaa näkyvien olevan ylä, etu ja oikea*/

const char ulatavu = 0x01;
const char etutavu = 0x02;
const char oiktavu = 0x04;
const char alatavu = 0x08;
const char taktavu = 0x10;
const char vastavu = 0x20;

kuutio_t* kuutio;
kuva_t* kuva;
#ifdef __KUUTION_KOMMUNIKOINTI__
int viimeViesti;
shmRak_s *ipc;
#endif

inline void __attribute__((always_inline)) hae_nakuvuus() {
  kuutio->nakuvat = 0;
  if((kuutio->rotX > 0 && fabs(kuutio->rotY) < PI/2) ||		\
     (kuutio->rotX < 0 && fabs(kuutio->rotY) > PI/2))
    kuutio->nakuvat |= ulatavu;
  else
    kuutio->nakuvat |= alatavu;
  if(kuutio->rotY < 0)
    kuutio->nakuvat |= oiktavu;
  else
    kuutio->nakuvat |= vastavu;
  char tmpx = 0;
  char tmpy = 0;
  if(fabs(kuutio->rotX) > PI/2)
    tmpx = 1;
  if(fabs(kuutio->rotY) > PI/2)
    tmpy = 1;
  if(tmpx ^ tmpy)
    kuutio->nakuvat |= taktavu;
  else
    kuutio->nakuvat |= etutavu;
}

kuutio_t* luo_kuutio(const unsigned char N) {
  const char sivuja = 6;
  vari varit[6];
  varit[_u] = VARI(255,255,255); //valkoinen
  varit[_f] = VARI(0,  220,  0); //vihreä
  varit[_r] = VARI(255,0  ,0  ); //punainen
  varit[_d] = VARI(255,255,0  ); //keltainen
  varit[_b] = VARI(0,  0,  255); //sininen
  varit[_l] = VARI(255,100,0  ); //oranssi
  
  kuutio = malloc(sizeof(kuutio_t));
  kuutio->sivuja = sivuja;
  kuutio->N = N;
  kuutio->rotX = PI/6;
  kuutio->rotY = -PI/6;
  hae_nakuvuus();
  kuutio->sivut = malloc(sivuja*sizeof(char*));
  kuutio->varit = malloc(sizeof(varit));
  kuutio->ratkaistu = 1;
  
  for(int i=0; i<sivuja; i++) {
    kuutio->varit[i] = varit[i];
    kuutio->sivut[i] = malloc(N*N);
    for(int j=0; j<N*N; j++)
      kuutio->sivut[i][j] = i;
  }
  return kuutio;
}

void* tuhoa_kuutio() {
  for(int i=0; i<kuutio->sivuja; i++) {
    free(kuutio->sivut[i]);
    kuutio->sivut[i] = NULL;
  }
  free(kuutio->sivut);
  kuutio->sivut = NULL;
  free(kuutio->varit);
  kuutio->varit = NULL;
  free(kuutio);
  return NULL;
}

kuva_t* suora_sivu_kuvaksi(const int sivu) {
  char* pohja = kuva->pohjat[sivu];
  int N = kuutio->N;
  int resol = kuva->res1;
  int resPala = resol/N;
  for(int palaI=0; palaI<N; palaI++) {
    for(int i=0; i<resPala; i++) {
      int iKoord = palaI*resPala + i;
      for(int palaJ=0; palaJ<N; palaJ++) {
	for(int j=0; j<resPala; j++) {
	  int jKoord = palaJ*resPala + j;
	  pohja[iKoord*resol+jKoord] = kuutio->sivut[sivu][palaI*N+palaJ];
	}
      }
    }
  }
  /*tehdään raot palojen välille*/
  return kuva;
}

typedef struct {
  float x;
  float y;
  float z;
} koordf;

void tee_koordtit() {
  float xsij0, ysij0, zsij0, cosx, sinx, cosy, siny;
  float x,y;
  float xrot = kuutio->rotX, yrot = kuutio->rotY;
  char nakuvat = kuutio->nakuvat;
  int pit = kuva->res1*kuva->res1;
  int res1 = kuva->res1;
  int res2 = res1/2;
  koordf ktit[pit];
  
  /*1. pyöräytys erikseen kullekin sivulle, 2. on sama kaikilla*/
  cosx = cosf(xrot);
  sinx = sinf(xrot);
  cosy = cosf(yrot);
  siny = sinf(yrot);

  for(int sivu=0; sivu<6; sivu++) {
    switch(sivu){
    case _u:
      if(!(nakuvat & ulatavu))
	continue;
      ysij0 = res2; xsij0 = -res2; zsij0 = -res2;
      /*x-pyöräytys, j-liikuttaa z-suunnassa*/
      for(int i=0; i<res1; i++) {
	for(int j=0; j<res1; j++) {
	  ktit[i*res1+j].x = (i+xsij0)*1.0;
	  ktit[i*res1+j].y = ysij0*cosx - (j+zsij0)*sinx;
	  ktit[i*res1+j].z = ysij0*sinx + (j+zsij0)*cosx;
	}
      }
      break;
    case _d:
      if(!(nakuvat & alatavu))
	continue;
      ysij0 = -res2; xsij0 = -res2; zsij0 = res2;
      /*x-pyöräytys, j-liikuttaa -z-suunnassa*/
      for(int i=0; i<res1; i++) {
	for(int j=0; j<res1; j++) {
	  ktit[i*res1+j].x = (i+xsij0)*1.0;
	  ktit[i*res1+j].y = ysij0*cosx - (-j+zsij0)*sinx;
	  ktit[i*res1+j].z = ysij0*sinx + (-j+zsij0)*cosx;
	}
      }
      break;
    case _r:
      if(!(nakuvat & oiktavu))
	continue;
      ysij0 = res2; xsij0 = res2; zsij0 = res2;
      /*x-pyöräytys, i-liikuttaa -z-suunnassa, j = -y*/
      for(int i=0; i<res1; i++) {
	for(int j=0; j<res1; j++) {
	  ktit[i*res1+j].x = xsij0 * 1.0;
	  ktit[i*res1+j].y = (-j+ysij0)*cosx - (-i+zsij0)*sinx;
	  ktit[i*res1+j].z = (-j+ysij0)*sinx + (-i+zsij0)*cosx;
	}
      }
      break;
    case _l:
      if(!(nakuvat & vastavu))
	continue;
      ysij0 = res2; xsij0 = -res2; zsij0 = -res2;
      for(int i=0; i<res1; i++) {
	for(int j=0; j<res1; j++) {
	  ktit[i*res1+j].x = xsij0 * 1.0;
	  ktit[i*res1+j].y = (-j+ysij0)*cosx - (i+zsij0)*sinx;
	  ktit[i*res1+j].z = (-j+ysij0)*sinx + (i+zsij0)*cosx;
	}
      }
      break;
    case _f:
      if(!(nakuvat & etutavu))
	continue;
      ysij0 = res2; xsij0 = -res2; zsij0 = res2;
      for(int i=0; i<res1; i++) {
	for(int j=0; j<res1; j++) {
	  ktit[i*res1+j].x = (i+xsij0)*1.0;
	  ktit[i*res1+j].y = (-j+ysij0)*cosx - zsij0*sinx;
	  ktit[i*res1+j].z = (-j+ysij0)*sinx + zsij0*cosx;
	}
      }
      break;
    case _b:
      if(!(nakuvat & taktavu))
	continue;
      ysij0 = res2; xsij0 = res2; zsij0 = -res2;
      for(int i=0; i<res1; i++) {
	for(int j=0; j<res1; j++) {
	  ktit[i*res1+j].x = (-i+xsij0)*1.0;
	  ktit[i*res1+j].y = (-j+ysij0)*cosx - zsij0*sinx;
	  ktit[i*res1+j].z = (-j+ysij0)*sinx + zsij0*cosx;
	}
      }
      break;
    } //switch
    
    /*2. y-pyöräytys, joka ymmärtääkseni pitäisi olla sama kaikilla,
      mutta ei näytä olevan, tämä on saatu kokeilemalla
      "koodi toimii, mutta en tiedä, miksi"*/
    switch(sivu) {
    case _u:
    case _l:
    case _f:
    for(int i=0; i<kuva->pit; i++) {
      x = ktit[i].x*cosy + ktit[i].z*siny;
      y = ktit[i].y;
      kuva->koordtit[sivu][i].x = (short)(x+kuva->sij0-xsij0);
      kuva->koordtit[sivu][i].y = (short)(-y+kuva->sij0+ysij0);
    }
    break;
    case _r:
    case _b:
      for(int i=0; i<kuva->pit; i++) {
	x = ktit[i].x*cosy + ktit[i].z*siny;
	y = ktit[i].y;
	kuva->koordtit[sivu][i].x = (short)(x+kuva->sij0-xsij0)+res1;
	kuva->koordtit[sivu][i].y = (short)(-y+kuva->sij0+ysij0);
      }
      break;
    case _d:
      for(int i=0; i<kuva->pit; i++) {
	x = ktit[i].x*cosy + ktit[i].z*siny;
	y = ktit[i].y;
	kuva->koordtit[sivu][i].x = (short)(x+kuva->sij0-xsij0);
	kuva->koordtit[sivu][i].y = (short)(-y+kuva->sij0+ysij0) + res1;
      }
      break;
    }
  }
}

void piirra_kuvaksi(const int sivu) {
  koord* ktit = kuva->koordtit[sivu];
  char* pohja = kuva->pohjat[sivu];
  for(int i=0; i<kuva->pit; i++) {
    short laji = pohja[i];
    vari vari = kuutio->varit[laji];
    SDL_SetRenderDrawColor(kuva->rend, vari.v[0], vari.v[1], vari.v[2], 255);
    SDL_RenderDrawPoint(kuva->rend, ktit[i].x, ktit[i].y);
  }
}

void paivita() {
  if(!kuva->paivita)
    return;
  kuva->paivita = 0;
  SDL_SetRenderDrawColor(kuva->rend, 0, 0, 0, 255);
  SDL_RenderClear(kuva->rend);
  
  if(kuutio->nakuvat & ulatavu)
    piirra_kuvaksi(_u);
  else
    piirra_kuvaksi(_d);
  if(kuutio->nakuvat & etutavu)
    piirra_kuvaksi(_f);
  else
    piirra_kuvaksi(_b);
  if(kuutio->nakuvat & oiktavu)
    piirra_kuvaksi(_r);
  else
    piirra_kuvaksi(_l);

  SDL_RenderPresent(kuva->rend);
}

void siirto(int puoli, char siirtokaista, char maara) {
  if(maara == 0)
    return;
  char N = kuutio->N;
  if(siirtokaista < 1 || siirtokaista > N)
    return;
  else if(siirtokaista == N) {
    maara = (maara+2) % 4;
    puoli = (puoli+3) % 6;
    siirtokaista = 1;
  }
  struct i4 {int i[4];} jarj;
  struct i4 kaistat;
  int a,b; //kaistat kahdelta eri puolelta katsottuna
  int kaista1,kaista2;
  char apu[N*N];
  char *sivu1, *sivu2;

  a = N-siirtokaista;
  b = (N-1)-a;  
  
  /*toiminta jaetaan kääntöakselin perusteella*/
  switch(puoli) {
  case _r:
  case _l:
    if(puoli == _r) {
      kaistat = (struct i4){{a,a,a,b}};
      jarj = (struct i4){{_u, _f, _d, _b}};
    } else {
      jarj = (struct i4){{_u, _b, _d, _f}};
      kaistat = (struct i4){{b,a,b,b}};
    }
    sivu1 = kuutio->sivut[jarj.i[0]];
    /*1. sivu talteen*/
    for(int i=0; i<N; i++)
      apu[i] = sivu1[N*kaistat.i[0]+i];
    /*siirretään*/
    for(int j=0; j<3; j++) {
      sivu1 = kuutio->sivut[jarj.i[j]];
      sivu2 = kuutio->sivut[jarj.i[j+1]];
      kaista1 = kaistat.i[j];
      kaista2 = kaistat.i[j+1];
      if( (j<2 && puoli == _l) || (j>=2 && puoli == _r) )
	for(int i=0; i<N; i++)
	  sivu1[N*kaista1+i] = sivu2[N*kaista2 + N-1-i];
      else
	for(int i=0; i<N; i++)
	  sivu1[N*kaista1+i] = sivu2[N*kaista2+i];
    }
    /*viimeinen*/
    sivu1 = kuutio->sivut[jarj.i[3]];
    if(puoli == _r)
      for(int i=0; i<N; i++)
	sivu1[N*kaistat.i[3]+i] = apu[N-1-i];
    else
      for(int i=0; i<N; i++)
	sivu1[N*kaistat.i[3]+i] = apu[i];
    break;

  case _d:
  case _u:
    if(puoli == _d) {
      kaistat = (struct i4){{a,a,a,a}};
      jarj = (struct i4){{_f, _l, _b, _r}};
    } else {
      jarj = (struct i4){{_f, _r, _b, _l}};
      kaistat = (struct i4){{b,b,b,b}};
    }
    sivu1 = kuutio->sivut[jarj.i[0]];
    /*1. sivu talteen*/
    for(int i=0; i<N; i++)
      apu[i] = sivu1[N*i + kaistat.i[0]];
    /*siirretään*/
    for(int j=0; j<3; j++) {
      sivu1 = kuutio->sivut[jarj.i[j]];
      sivu2 = kuutio->sivut[jarj.i[j+1]];
      kaista1 = kaistat.i[j];
      kaista2 = kaistat.i[j+1];
      for(int i=0; i<N; i++)
	sivu1[N*i + kaista1] = sivu2[N*i + kaista2];
    }
    /*viimeinen*/
    sivu1 = kuutio->sivut[jarj.i[3]];
    for(int i=0; i<N; i++)
      sivu1[N*i+kaistat.i[3]] = apu[i];
    break;

  case _f:
  case _b:
    if(puoli == _f) {
      kaistat = (struct i4){{a,a,b,b}};
      jarj = (struct i4){{_u, _l, _d, _r}};
    } else {
      kaistat = (struct i4){{b,a,a,b}};
      jarj = (struct i4){{_u, _r, _d, _l}};
    }
    sivu1 = kuutio->sivut[jarj.i[0]];
    /*1. sivu talteen*/
    for(int i=0; i<N; i++)
      apu[i] = sivu1[N*i + kaistat.i[0]];
    /*siirretään, tässä vuorottelee, mennäänkö i- vai j-suunnassa*/
    for(int j=0; j<3; j++) {
      sivu1 = kuutio->sivut[jarj.i[j]];
      sivu2 = kuutio->sivut[jarj.i[j+1]];
      kaista1 = kaistat.i[j];
      kaista2 = kaistat.i[j+1];
      if(j % 2 == 0) //u tai d alussa
	if(puoli == _f) {
	  for(int i=0; i<N; i++) //1: i-suunta, 2: j-suunta
	    sivu1[N*i + kaista1] = sivu2[N*kaista2 + N-1-i];
	} else {
	  for(int i=0; i<N; i++) //1: i-suunta, 2: j-suunta
	    sivu1[N*i + kaista1] = sivu2[N*kaista2 + i];
	}
      else
	if(puoli == _b) {
	  for(int i=0; i<N; i++) //1: j-suunta, 2:i-suunta
	    sivu1[N*kaista1 + i] = sivu2[N*(N-1-i) + kaista2];
	} else {
	  for(int i=0; i<N; i++) //1: j-suunta, 2:i-suunta
	    sivu1[N*kaista1 + i] = sivu2[N*i + kaista2];
	}
    }
    /*viimeinen*/
    sivu1 = kuutio->sivut[jarj.i[3]];
    if(puoli == _f)
      for(int i=0; i<N; i++)
	sivu1[N*kaistat.i[3]+i] = apu[i];
    else
      for(int i=0; i<N; i++)
	sivu1[N*kaistat.i[3]+i] = apu[N-1-i];
    break;
  default:
    break;
  }

  /*siivusiirrolle (slice move) kääntö on nyt suoritettu*/
  if(siirtokaista > 1 && siirtokaista < N) {
    siirto(puoli, siirtokaista, maara-1);
    return;
  }
  
  /*käännetään käännetty sivu*/
  int sivuInd;
  switch(puoli) {
  case _r:
    sivuInd = _r;
    if(siirtokaista == N)
      sivuInd = _l;
    break;
  case _l:
    sivuInd = _l;
    if(siirtokaista == N)
      sivuInd = _r;
    break;
  case _u:
    sivuInd = _u;
    if(siirtokaista == N)
      sivuInd = _d;
    break;
  case _d:
    sivuInd = _d;
    if(siirtokaista == N)
      sivuInd = _u;
    break;
  case _b:
    sivuInd = _b;
    if(siirtokaista == N)
      sivuInd = _f;
    break;
  case _f:
    sivuInd = _f;
    if(siirtokaista == N)
      sivuInd = _b;
    break;
  default:
    return;
  }
  char* sivu = kuutio->sivut[sivuInd];
  /*kopioidaan kys. sivu ensin*/
  for(int i=0; i<N*N; i++)
    apu[i] = sivu[i];

  /*nyt käännetään: */
#define arvo(sivu,j,i) sivu[i*N+j]
  for(int i=0; i<N; i++)
    for(int j=0; j<N; j++)
      arvo(sivu,j,i) = arvo(apu,N-1-i,j);
#undef arvo
  siirto(puoli, siirtokaista, maara-1);
}

void kaanto(char akseli, char maara) {
  if(!maara)
    return;
  if(maara < 0)
    maara += 4;
  char sivu;
  switch(akseli) {
  case 'x':
    sivu = _r;
    break;
  case 'y':
    sivu = _u;
    break;
  case 'z':
    sivu = _f;
    break;
  default:
    return;
  }
  for(char i=1; i<=kuutio->N; i++)
    siirto(sivu, i, 1);
  
  kaanto(akseli, maara-1);
  return;
}

inline char onkoRatkaistu() {
  for(int sivu=0; sivu<6; sivu++) {
    char laji = kuutio->sivut[sivu][0];
    for(int i=1; i<kuutio->N*kuutio->N; i++)
      if(kuutio->sivut[sivu][i] != laji)
	return 0;
  }
  return 1;
}

inline void __attribute__((always_inline)) kaantoInl(char akseli, char maara) {
  kaanto(akseli, maara);
  for(int i=0; i<6; i++)
    suora_sivu_kuvaksi(i);
  kuva->paivita = 1;
}

inline void __attribute__((always_inline)) siirtoInl(int puoli, char kaista, char maara) {
  siirto(puoli, kaista, maara);
  for(int i=0; i<6; i++)
    suora_sivu_kuvaksi(i);
  kuva->paivita = 1;
  kuutio->ratkaistu = onkoRatkaistu();
#ifdef __KUUTION_KOMMUNIKOINTI__
  if(viimeViesti == ipcTarkastelu) {
    ipc->viesti = ipcAloita;
    viimeViesti = ipcAloita;
  } else if(viimeViesti == ipcAloita && kuutio->ratkaistu) {
    ipc->viesti = ipcLopeta;
    viimeViesti = ipcLopeta;
  }
#endif
}

int main(int argc, char** argv) {
  char ohjelman_nimi[] = "Kuutio";
  int ikkuna_x = 300;
  int ikkuna_y = 300;
  int ikkuna_w = 500;
  int ikkuna_h = 500;
  char N;
  char oli_sdl = 0;
  if(argc < 2)
    N = 3;
  else
    sscanf(argv[1], "%hhu", &N);
  
  if(!SDL_WasInit(SDL_INIT_VIDEO))
    SDL_Init(SDL_INIT_VIDEO);
  else
    oli_sdl = 1;
  kuutio = luo_kuutio(N);

  /*Kuvan tekeminen*/
  kuva = malloc(sizeof(kuva_t));
  kuva->ikkuna = SDL_CreateWindow\
    (ohjelman_nimi, ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h, SDL_WINDOW_RESIZABLE);
  kuva->rend = SDL_CreateRenderer(kuva->ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);
  if(!kuva->rend)
    goto ULOS;
  kuva->xRes = ikkuna_w;
  kuva->yRes = ikkuna_h;
  kuva->paivita = 1;
  kuva->res1 = (ikkuna_h < ikkuna_w)? ikkuna_h/sqrt(3.0) : ikkuna_w/sqrt(3.0);
  kuva->sij0 = (ikkuna_h < ikkuna_w)? (ikkuna_h-kuva->res1)/2: (ikkuna_w-kuva->res1)/2;
  kuva->pit = kuva->res1*kuva->res1;
  kuva->pohjat = malloc(6*sizeof(char*));
  for(int i=0; i<6; i++) {
    kuva->pohjat[i] = malloc(kuva->res1*kuva->res1);
    suora_sivu_kuvaksi(i);
  }
  kuva->koordtit = malloc(6*sizeof(koord*));
  for(int i=0; i<6; i++)
    kuva->koordtit[i] = malloc(kuva->pit*sizeof(koord));
  tee_koordtit();

#ifdef __KUUTION_KOMMUNIKOINTI__
  ipc = liity_muistiin();
  if(!ipc)
    return 1;
#endif

#define siirtoInl1(puoli, maara) siirtoInl(puoli, siirtokaista, maara)
  
  SDL_Event tapaht;
  int xVanha, yVanha;
  char hiiri_painettu = 0;
  char siirtokaista = 1;
  while(1) {
    while(SDL_PollEvent(&tapaht)) {
      switch(tapaht.type) {
      case SDL_QUIT:
	goto ULOS;
      case SDL_KEYDOWN:
	switch(tapaht.key.keysym.scancode) {
	case SDL_SCANCODE_I:
	  siirtoInl1(_u, 1);
	  break;
	case SDL_SCANCODE_L:
	  siirtoInl1(_r, 1);
	  break;
	case SDL_SCANCODE_J:
	  siirtoInl1(_r, 3);
	  break;
	case SDL_SCANCODE_PERIOD:
	  siirtoInl1(_d, 3);
	  break;
	case SDL_SCANCODE_K:
	  siirtoInl1(_f, 1);
	  break;
	case SDL_SCANCODE_O:
	  siirtoInl1(_b, 3);
	  break;
	case SDL_SCANCODE_E:
	  siirtoInl1(_u, 3);
	  break;
	case SDL_SCANCODE_F:
	  siirtoInl1(_l, 1);
	  break;
	case SDL_SCANCODE_S:
	  siirtoInl1(_l, 3);
	  break;
	case SDL_SCANCODE_X:
	  siirtoInl1(_d, 1);
	  break;
	case SDL_SCANCODE_D:
	  siirtoInl1(_f, 3);
	  break;
	case SDL_SCANCODE_W:
	  siirtoInl1(_b, 1);
	  break;
	case SDL_SCANCODE_H:
	  kaantoInl('y', 1);
	  break;
	case SDL_SCANCODE_G:
	  kaantoInl('y', 3);
	  break;
	default:
	  switch(tapaht.key.keysym.sym) {
	  case SDLK_RSHIFT:
	  case SDLK_LSHIFT:
	    siirtokaista++;
	    break;
	    /*käytetään kääntämisten määrässä siirtokaistaa*/
	  case SDLK_SPACE:
	    kaantoInl('y', siirtokaista);
	    break;
	  case SDLK_RETURN:
	    kaantoInl('x', siirtokaista);
	    break;
	  case SDLK_BACKSPACE:
	    kaantoInl('z', siirtokaista);
	    break;
#ifdef __KUUTION_KOMMUNIKOINTI__
	  case SDLK_F1:
	    lue_siirrot(ipc);
	    ipc->viesti = ipcTarkastelu;
	    viimeViesti = ipcTarkastelu;
	    break;
#endif
	  default:
	    if('1' < tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9')
	      siirtokaista += tapaht.key.keysym.sym - '1';
	    else if(SDLK_KP_1 < tapaht.key.keysym.sym && tapaht.key.keysym.sym <= SDLK_KP_9)
	      siirtokaista += tapaht.key.keysym.sym - SDLK_KP_1;
	    break;
	  }
	  break;
	}
	break;
      case SDL_KEYUP:
	switch(tapaht.key.keysym.sym) {
	case SDLK_RSHIFT:
	case SDLK_LSHIFT:
	  siirtokaista = 1;
	  break;
	default:
	  if('1' <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9')
	    siirtokaista = 1;
	  else if(SDLK_KP_1 <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= SDLK_KP_9)
	    siirtokaista = 1;
	  break;
	}
	break;
      case SDL_MOUSEBUTTONDOWN:
	xVanha = tapaht.button.x;
	yVanha = tapaht.button.y;
	hiiri_painettu = 1;
	break;
      case SDL_MOUSEMOTION:;
	/*pyöritetään, raahauksesta hiirellä*/
	if(hiiri_painettu) {
	  float xEro = tapaht.motion.x - xVanha; //vasemmalle negatiivinen
	  float yEro = tapaht.motion.y - yVanha; //alas positiivinen
	  xVanha = tapaht.motion.x;
	  yVanha = tapaht.motion.y;
	  kuutio->rotY += xEro*PI/(2*kuva->res1);
	  kuutio->rotX += yEro*PI/(2*kuva->res1);
	  if(kuutio->rotY < -PI)
	    kuutio->rotY += 2*PI;
	  else if (kuutio->rotY > PI)
	    kuutio->rotY -= 2*PI;
	  if(kuutio->rotX < -PI)
	    kuutio->rotX += 2*PI;
	  else if (kuutio->rotX > PI)
	    kuutio->rotX -= 2*PI;
	  
	  hae_nakuvuus();
	  tee_koordtit();
	  kuva->paivita = 1;
	}
	break;
      case SDL_MOUSEBUTTONUP:
	hiiri_painettu = 0;
	break;
      }
    }
    paivita(kuutio, kuva);
    SDL_Delay(0.02);
  }
  
 ULOS:
  for(int i=0; i<6; i++)
    free(kuva->pohjat[i]);
  free(kuva->pohjat);
  for(int i=0; i<6; i++)
    free(kuva->koordtit[i]);
  free(kuva->koordtit);
  SDL_DestroyRenderer(kuva->rend);
  SDL_DestroyWindow(kuva->ikkuna);
  free(kuva);
  kuutio = tuhoa_kuutio(kuutio);
  if(!oli_sdl)
    SDL_Quit();
  return 0;
}
