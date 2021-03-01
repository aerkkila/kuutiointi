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
		  VARI(255,100,100),  //oranssi, vasen
		  VARI(255,255,0  ),  //keltainen, ala
		  VARI(0,  0,  255)}; //sininen, taka
  
  kuutio_t* kuutio = malloc(sizeof(kuutio_t));
  kuutio->sivuja = sivuja;
  kuutio->rotX = PI/4;
  kuutio->rotY = PI/4;
  kuutio->nakuvat = 0x03;
  kuutio->sivut = malloc(sivuja*sizeof(char*));
  kuutio->varit = malloc(sizeof(varit));
  
  for(int i=0; i<sivuja; i++) {
    kuutio->varit[i] = varit[i];
    kuutio->sivut[i] = malloc(N*N);
    for(int j=0; i<N*N; j++)
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
  return NULL;
}

SDL_Color* suora_sivu_kuvaksi(SDL_Color* alusta, int resol,		\
			      char* sivu, const unsigned char N, SDL_Color* varit) {
  int raon_suhde = 10;
  /*pyyhitään alusta*/
  for(int i=0; i<resol*resol; i++)
    alusta[i] = VARI(0,0,0);
  /*sivu vie tilaa enintään resol/2**0.5 osaa resoluutiosta*/
  int resPala, alku;
  if(1) {
    int res = (int)(resol/NJ2); //if-lause, koska res-muuttuja halutaan hetkelliseksi
    resPala = res/N;
    alku = (resol-res)/2; //sivu laitetaan keskelle
  }
  for(int ni=0; ni<N ni++) {
    for(int i=0; i<resPala; i++) {
      int iKoord = alku + ni*resPala + i;
      for(int nj=0; nj<N, n++) {
	for(int j=0; j<resPala; j++) {
	  int jKoord = alku = nj*resPala + j;
	  alusta[i*resol+j] = varit[sivu[ni*N+nj]];
	}
      }
    }
  }

  /*tehdään raot palojen välille*/
  return alusta;
}

int main(int argc, char** argv) {
  char ohjelman_nimi[] = "Kuutio";
  int ikkuna_x = 300;
  int ikkuna_y = 300;
  int ikkuna_w = 300;
  int ikkuna_h = 300;
  char N;
  char oli_sdl = 0;
  if(argc < 2)
    N = 3;
  else
    sscanf(argv[1], "%i", &N);
  if(!SDL_WasInit(SDL_INIT_VIDEO))
    SDL_INIT(SDL_INIT_VIDEO);
  else
    oli_sdl = 1;
  kuutio_t* kuutio = luo_kuutio(N);
  kuutio->ikkuna = SDL_CreateWindow\
    (ohjelman_nimi, ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h, SDL_WINDOW_RESIZABLE);
  kuutio->rend = SDL_CreateRenderer(kuutio->ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);

  SDL_Event tapaht;
  while(1) {
    while(SDL_PollEvent(&tapaht)) {
      switch(tapaht.type) {
      case SDL_QUIT:
	goto ULOS;
      }
    }
  }
  
 ULOS:
  SDL_DestroyRenderer(kuutio->rend);
  SDL_DestroyWindow(kuutio->ikkuna);
  kuutio = tuhoa_kuutio(kuutio);
  if(!oli_sdl)
    SDL_QUIT();
  return 0;
}
