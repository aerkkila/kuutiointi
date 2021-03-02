#include <stdio.h>
#include <SDL.h>
#include <stdlib.h>
#include <math.h>
#include "kuutio.h"

#define PI 3.14159265358979

/*näkyvät sivut määritellään tavuna, jossa bitit oikealta alkaen ovat
  ylä, etu, oikea, vasen, pohja, taka;
  esim 0x03 tarkoittaa näkyvien olevan ylä, etu ja oikea*/

kuutio_t* luo_kuutio(const unsigned char N) {
  const char sivuja = 6;
  vari varit[] = {VARI(255,255,255),  //valkoinen, ylä
		  VARI(0,  220,  0),  //vihreä, etu
		  VARI(255,0  ,0  ),  //punainen, oikea
		  VARI(255,100,0  ),  //oranssi, vasen
		  VARI(255,255,0  ),  //keltainen, ala
		  VARI(0,  0,  255),  //sininen, taka
		  VARI(0  ,0,  0  )}; //taustaväri
  
  kuutio_t* kuutio = malloc(sizeof(kuutio_t));
  kuutio->sivuja = sivuja;
  kuutio->N = N;
  kuutio->rotX = -PI/4*0;
  kuutio->rotY = PI/4*0;
  kuutio->nakuvat = 0x03;
  kuutio->sivut = malloc(sivuja*sizeof(char*));
  kuutio->varit = malloc(sizeof(varit));
  
  for(int i=0; i<sivuja; i++) {
    kuutio->varit[i] = varit[i];
    kuutio->sivut[i] = malloc(N*N);
    for(int j=0; j<N*N; j++)
      kuutio->sivut[i][j] = i;
  }
  return kuutio;
}

void* tuhoa_kuutio(kuutio_t* kuutio) {
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

kuva_t* suora_sivu_kuvaksi(kuva_t* kuva, kuutio_t* kuutio, const int sivu) {
  //int raon_suhde = 10;
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

/*tässä olisi parempi hakea pyöräytysmatriisilla vain ääriviivat
  ja toimi niitten välillä for-silmukoilla
  tämä olisi tehokkaampaa ja jokainen piste värjättäisiin*/
#if 0
kuva_t* kaanna_sivua_tasolla(kuva_t* kuva, kuutio_t* kuutio, float kulma) {
  int xRes = kuva->xRes, yRes = kuva->yRes, res1 = kuva->res1;
  float sin = sinf(kulma);
  float cos = cosf(kulma);
  int xSiirt = (int)(res1*sinf(kuutio->rotY));
  int x, y;
  for(int i=0; i<kuva->res1; i++) {
    for(int j=0; j<kuva->res1; j++) {
      x = (int)(cos*i-sin*j)+xSiirt;
      y = (int)(sin*i+cos*j);
      if(x<0 || y<0 || x>=xRes || y>=yRes)
	continue;
      kuva->pohja2[x*yRes+y] = kuva->pohja1[i*res1+j];
      if(x<xRes-1)
	kuva->pohja2[x*yRes+y+1] = kuva->pohja1[i*res1+j]; //vältetään mustat pisteet
    }
  }
  return kuva;
}
#endif

typedef struct {
  float x;
  float y;
  float z;
} koordf;

/*y-pyöräytys, joka tehdään toisena*/
inline void __attribute__((always_inline)) y_puorautus(kuva_t* kuva,	\
						       char sivu,	\
						       koordf* ktit,	\
						       float xsij0, float ysij0, \
						       float cosx, float cosy) {
  
  
}

void tee_koordtit(kuva_t* kuva, float xrot, float yrot) {
  float xsij0, ysij0, zsij0, cosx, sinx, cosy, siny;
  float x,y;
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
      /*yläpinta*/
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
      /*alapinta*/
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
      /*oikea*/
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
      /*vasen: j = -y, i = z*/
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
      /*etuosa: j=-y, i=x*/
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
      /*takapinta: j=-y, i=-x*/
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
    
    /*y-pyöräytys, joka on sama kaikilla*/
    for(int i=0; i<kuva->pit; i++) {
      x = ktit[i].x*cosy + ktit[i].z*siny;
      y = ktit[i].y;
      kuva->koordtit[sivu][i].x = (short)(x+kuva->sij0-xsij0);
      kuva->koordtit[sivu][i].y = (short)(-y+kuva->sij0+ysij0);
    }
  }
}
  
void piirra_kuvaksi(kuva_t* kuva, kuutio_t* kuutio, const int sivu) {
  koord* ktit = kuva->koordtit[sivu];
  char* pohja = kuva->pohjat[sivu];
  for(int i=0; i<kuva->pit; i++) {
    short laji = pohja[i];
    vari vari = kuutio->varit[laji];
    SDL_SetRenderDrawColor(kuva->rend, vari.v[0], vari.v[1], vari.v[2], 255);
    SDL_RenderDrawPoint(kuva->rend, ktit[i].x, ktit[i].y);
  }
}

void paivita(kuutio_t* kuutio, kuva_t* kuva) {
  if(!kuva->paivita)
    return;
  kuva->paivita = 0;
  SDL_SetRenderDrawColor(kuva->rend, 0, 0, 0, 255);
  SDL_RenderClear(kuva->rend);
  
  if(kuutio->nakuvat & 0x01)
    piirra_kuvaksi(kuva, kuutio, _u);
  else
    piirra_kuvaksi(kuva, kuutio, _d);
  if(kuutio->nakuvat & 0x02)
    piirra_kuvaksi(kuva, kuutio, _f);
  else
    piirra_kuvaksi(kuva, kuutio, _b);
  if(kuutio->nakuvat & 0x04)
    piirra_kuvaksi(kuva, kuutio, _r);
  else
    piirra_kuvaksi(kuva, kuutio, _l);

  SDL_RenderPresent(kuva->rend);
}

void siirto(kuutio_t* kuutio, char puoli, char maara) {
  if(maara == 0)
    return;
  char N = kuutio->N;
  struct i4 {int i[4];} jarj;
  struct i4 kaistat;
  int a,b;
  int kaista1,kaista2;
  char apu[N*N];
  char *sivu1, *sivu2;
  
  /*toiminta jaetaan kääntöakselin perusteella*/
  switch(puoli) {
  case 'r':
  case 'l':
    if(puoli == 'r') {
      a = N-1;
      b = (N-1)-a;
      kaistat = (struct i4){{a,a,a,b}};
      jarj = (struct i4){{_u, _f, _d, _b}};
    } else {
      a = 0;
      b = (N-1)-a;
      jarj = (struct i4){{_u, _b, _d, _f}};
      kaistat = (struct i4){{a,b,a,a}};
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
      for(int i=0; i<N; i++)
	sivu1[N*kaista1+i] = sivu2[N*kaista2+i];
    }
    /*viimeinen*/
    sivu1 = kuutio->sivut[jarj.i[3]];
    for(int i=0; i<N; i++)
      sivu1[N*kaistat.i[3]+i] = apu[i];
    break;

  case 'd':
  case 'u':
    if(puoli == 'd') {
      a = N-1;
      b = (N-1)-a;
      kaistat = (struct i4){{a,a,a,a}};
      jarj = (struct i4){{_f, _l, _b, _r}};
    } else {
      a = 0;
      b = (N-1)-a;
      jarj = (struct i4){{_f, _r, _b, _l}};
      kaistat = (struct i4){{a,a,a,a}};
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

  case 'b':
  default:
    return;
  }
  
  /*käännetään käännetty sivu*/
  int sivuInd;
  switch(puoli) {
  case 'r':
    sivuInd = _r;
    break;
  case 'l':
    sivuInd = _l;
    break;
  case 'u':
    sivuInd = _u;
    break;
  case 'd':
    sivuInd = _d;
    break;
  case 'b':
    sivuInd = _b;
    break;
  case 'f':
    sivuInd = _f;
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
  siirto(kuutio, puoli, maara-1);
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
  kuutio_t* kuutio = luo_kuutio(N);

  /*Kuvan tekeminen*/
  kuva_t* kuva = malloc(sizeof(kuva_t));
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
    suora_sivu_kuvaksi(kuva, kuutio, i);
  }
  kuva->koordtit = malloc(6*sizeof(koord*));
  for(int i=0; i<6; i++)
    kuva->koordtit[i] = malloc(kuva->pit*sizeof(koord));
  tee_koordtit(kuva, kuutio->rotX, kuutio->rotY);


  SDL_Event tapaht;
  int xVanha, yVanha;
  char hiiri_painettu = 0;
  while(1) {
    while(SDL_PollEvent(&tapaht)) {
      switch(tapaht.type) {
      case SDL_QUIT:
	goto ULOS;
      case SDL_KEYDOWN:
	switch(tapaht.key.keysym.sym) {
	case 'r':
	case 'l':
	case 'u':
	case 'd':
	case 'b':
	case 'f':
	  siirto(kuutio, tapaht.key.keysym.sym, 1);
	  for(int i=0; i<6; i++)
	    suora_sivu_kuvaksi(kuva, kuutio, i);
	  kuva->paivita = 1;
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
	  kuutio->rotY -= xEro*PI/(2*kuva->res1);
	  kuutio->rotX += yEro*PI/(2*kuva->res1);
	  if(kuutio->rotY < -PI)
	    kuutio->rotY += 2*PI;
	  else if (kuutio->rotY > PI)
	    kuutio->rotY -= 2*PI;
	  if(kuutio->rotX < -PI)
	    kuutio->rotX += 2*PI;
	  else if (kuutio->rotX > PI)
	    kuutio->rotX -= 2*PI;

	  /*näkyvyys*/
	  kuutio->nakuvat = 0;
	  if(kuutio->rotX > 0)
	    kuutio->nakuvat |= 0x01; //yläosa
	  else
	    kuutio->nakuvat |= 0x10; //pohja
	  if(kuutio->rotY < 0)
	    kuutio->nakuvat |= 0x04; //oikea
	  else
	    kuutio->nakuvat |= 0x08; //vasen
	  char tmpx = 0;
	  char tmpy = 0;
	  if(fabs(kuutio->rotX) > PI/2)
	    tmpx = 1;
	  if(fabs(kuutio->rotY) > PI/2)
	    tmpy = 1;
	  if(tmpx ^ tmpy)
	    kuutio->nakuvat |= 0x20; //takapinta
	  else
	    kuutio->nakuvat |= 0x02; //etupinta
	  tee_koordtit(kuva, kuutio->rotX, kuutio->rotY);
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
  SDL_DestroyRenderer(kuva->rend);
  SDL_DestroyWindow(kuva->ikkuna);
  free(kuva);
  kuutio = tuhoa_kuutio(kuutio);
  if(!oli_sdl)
    SDL_Quit();
  return 0;
}
