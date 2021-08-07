#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include "asetelma.h"

int kaunnista();
int lue_tiedosto(const char* tiednimi, char* rajaus);

/*alustaa grafiikan ja ikkunan ja renderin yms ja lataa fontit,
  käynnistää käyttöliittymän*/

char* apuc;

int main(int argc, char** argv) {
  setlocale(LC_ALL, "fi_FI.utf8");
  chdir(uloskansio);
  int r = 0;
  
  /*grafiikan alustaminen*/
  if (SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "Virhe: Ei voi alustaa SDL-grafiikkaa: %s\n", SDL_GetError());
    r = 1;
    goto EI_SDL;
  }
  if (TTF_Init()) {
    fprintf(stderr, "Virhe: Ei voi alustaa SDL_ttf-fonttikirjastoa: %s\n", TTF_GetError());
    SDL_Quit();
    r = 1;
    goto EI_TTF;
  }
  ikkuna = SDL_CreateWindow\
    (ohjelman_nimi, ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h, SDL_WINDOW_RESIZABLE);
  rend = SDL_CreateRenderer(ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);

  apuc = malloc(1500);
  if(asetelma())
    goto EI_FONTTI;
  
  /*valintaolion kuvat*/
  SDL_Surface* kuva;
  if(!(kuva = SDL_LoadBMP(url_valittu))) {
    fprintf(stderr, "Virhe: Ei luettu kuvaa \"%s\"\n", url_valittu);
    goto EI_FONTTI;
  }
  tarknap.kuvat.valittu = SDL_CreateTextureFromSurface(rend, kuva);
  SDL_FreeSurface(kuva);
  if(!(kuva = SDL_LoadBMP(url_eivalittu))) {
    fprintf(stderr, "Virhe: Ei luettu kuvaa \"%s\"\n", url_eivalittu);
    goto EI_FONTTI;
  }
  tarknap.kuvat.ei_valittu = SDL_CreateTextureFromSurface(rend, kuva);
  SDL_FreeSurface(kuva);
  
  time_t t;
  srand((unsigned) time(&t));
  strcpy(kellool.teksti, " ");

  SDL_RenderClear(rend);
  SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);

  /*luetaan tiedosto tarvittaessa*/
  /*luettavaa voidaan rajata:
    viimeiset 1000 --> -1000:
    alkaen 1000:sta --> 1000:
    1000 ensimmäistä --> :1000
    alkaen 1000:sta viimeiseen 1000:en asti --> 1000:-1000 jne*/
  if(argc > 2) {
    if( lue_tiedosto(argv[1], argv[2]) )
      return 1;
  } else if(argc > 1) {
    if( lue_tiedosto(argv[1], "") )
      return 1;
  }
  
  r = kaunnista();

  tuhoa_asetelma();
 EI_FONTTI:
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(ikkuna);
  TTF_Quit();
 EI_TTF:
  SDL_Quit();
 EI_SDL:
  return r;
}
