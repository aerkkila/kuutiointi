#include <stdio.h>
#include <SDL.h>
#include <stdlib.h>
#include <math.h>
#include "kuutio.h"

#define PI 3.14159265358979
#define NJ2 1.414213562

/*näkyvät sivut määritellään tavuna, jossa bitit oikealta alkaen ovat
  ylä, etu oikea, vasen, pohja, taka;
  esim 0x03 tarkoittaa näkyvien olevan ylä, etu ja oikea*/

kuutio_t* luo_kuutio(const unsigned char N) {
  const char sivuja = 6;
  vari varit[] = {VARI(255,255,255),  //valkoinen, ylä
		  VARI(0,  255,0  ),  //vihrea, etu
		  VARI(255,0  ,0  ),  //punainen, oikea
		  VARI(255,100,0  ),  //oranssi, vasen
		  VARI(255,255,0  ),  //keltainen, ala
		  VARI(0,  0,  255),  //sininen, taka
		  VARI(0  ,0,  0  )}; //taustaväri
  
  kuutio_t* kuutio = malloc(sizeof(kuutio_t));
  kuutio->sivuja = sivuja;
  kuutio->N = N;
  kuutio->rotX = PI/4;
  kuutio->rotY = PI/4;
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

kuva_t* suora_sivu_kuvaksi(kuva_t* kuva, kuutio_t* kuutio, const unsigned char ind) {
  int raon_suhde = 10;
  /*pyyhitään alusta*/
  for(int i=0; i<kuva->xRes*kuva->yRes; i++)
    kuva->pohja[i] = 6;
  /*sivu vie tilaa enintään resol/2**0.5 osaa resoluutiosta*/
  int N = kuutio->N;
  int resol = kuva->yRes;
  int resPala, alku;
  if(1) {
    int res = (int)(resol/NJ2); //if-lause, koska res-muuttuja halutaan hetkelliseksi
    resPala = res/N;
    alku = (resol-res)/2; //sivu laitetaan keskelle
  }
  for(int palaI=0; palaI<N; palaI++) {
    for(int i=0; i<resPala; i++) {
      int iKoord = alku + palaI*resPala + i;
      for(int palaJ=0; palaJ<N; palaJ++) {
	for(int j=0; j<resPala; j++) {
	  int jKoord = 0 + palaJ*resPala + j;
	  kuva->pohja[iKoord*resol+jKoord] = kuutio->sivut[ind][palaI*N+palaJ];
	}
      }
    }
  }
  /*tehdään raot palojen välille*/
  return kuva;
}

void paivita(kuutio_t* kuutio, kuva_t* kuva) {
  if(!kuva->paivita)
    return;
  kuva->paivita = 0;
  suora_sivu_kuvaksi(kuva, kuutio, 1);
  for(int i=0; i<kuva->xRes; i++)
    for(int j=0; j<kuva->yRes; j++) {
      vari vari = kuutio->varit[(int)kuva->pohja[i*kuva->yRes+j]];
      SDL_SetRenderDrawColor(kuva->rend, vari.v[0], vari.v[1], vari.v[2], 255);
      SDL_RenderDrawPoint(kuva->rend, i, j);
    }
  SDL_RenderPresent(kuva->rend);
}

void siirto(kuutio_t* kuutio, char puoli, char maara) {
  if(maara == 0)
    return;
  char N = kuutio->N;
  struct jarj {int i[4];} jarj;
  char apu[N*N];
  char kaista=0;
  char *sivu1, *sivu2;
  
  /*toiminta jaetaan kääntöakselin perusteella*/
  switch(puoli) {
  case 'r':
    kaista = N-1;
  case 'l':;
    jarj = (struct jarj){{_u, _f, _d, _b}};
    sivu1 = kuutio->sivut[jarj.i[0]];
    /*1. sivu talteen*/
    for(int i=0; i<N; i++)
      apu[i] = sivu1[N*kaista+i];
    /*siirretään*/
    for(int j=0; j<3; j++) {
      sivu1 = kuutio->sivut[jarj.i[j]];
      sivu2 = kuutio->sivut[jarj.i[j+1]];
      for(int i=0; i<N; i++)
	sivu1[N*kaista+i] = sivu2[N*kaista+i];
    }
    /*viimeinen*/
    sivu1 = kuutio->sivut[jarj.i[3]];
    for(int i=0; i<N; i++)
      sivu1[N*kaista+i] = apu[i];
    break;
    
  case 'd':
    kaista = N-1;
  case 'u':
    jarj = (struct jarj){{_f, _r, _b, _l}};
    sivu1 = kuutio->sivut[jarj.i[0]];
    /*1. sivu talteen*/
    for(int i=0; i<N; i++)
      apu[i] = sivu1[N*i+kaista];
    /*siirretään*/
    for(int j=0; j<3; j++) {
      sivu1 = kuutio->sivut[jarj.i[j]];
      sivu2 = kuutio->sivut[jarj.i[j+1]];
      for(int i=0; i<N; i++)
	sivu1[N*i+kaista] = sivu2[N*i+kaista];
    }
    /*viimeinen*/
    sivu1 = kuutio->sivut[jarj.i[3]];
    for(int i=0; i<N; i++)
      sivu1[N*i+kaista] = apu[i];
    break;
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
#define arvo(sivu,i,j) sivu[i*N+j]
  N--; //kuvaa nyt suurinta indeksiä
  for(int i=0; i<N; i++)
    for(int j=0; j<N; j++)
      arvo(sivu,i,j) = arvo(apu,N-j, i);
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
  kuva->pohja = malloc(sizeof(char)*ikkuna_w*ikkuna_h);
  kuva->xRes = ikkuna_w;
  kuva->yRes = ikkuna_h;
  kuva->paivita = 1;

  SDL_Event tapaht;
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
	  kuva->paivita = 1;
	  break;
	}
      }
    }
    paivita(kuutio, kuva);
    SDL_Delay(0.02);
  }
  
 ULOS:
  free(kuva->pohja);
  SDL_DestroyRenderer(kuva->rend);
  SDL_DestroyWindow(kuva->ikkuna);
  free(kuva);
  kuutio = tuhoa_kuutio(kuutio);
  if(!oli_sdl)
    SDL_Quit();
  return 0;
}
