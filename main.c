#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <time.h>
#include "asetelma.h"

int kaunnista();

/*alustaa grafiikan ja ikkunan ja renderin yms ja lataa fontit,
  käynnistää käyttöliittymän*/

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

  if(asetelma())
    goto EI_FONTTI;
  
  /*valintaolion kuvat*/
  SDL_Surface* kuva;
  if(!(kuva = SDL_LoadBMP(url_valittu))) {
    fprintf(stderr, "Virhe: Ei luettu kuvaa \"%s\"\n", url_valittu);
    goto EI_KUVAA;
  }
  tarknap.kuvat.valittu = SDL_CreateTextureFromSurface(rend, kuva);
  SDL_FreeSurface(kuva);
  if(!(kuva = SDL_LoadBMP(url_eivalittu))) {
    fprintf(stderr, "Virhe: Ei luettu kuvaa \"%s\"\n", url_eivalittu);
    goto EI_KUVAA;
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

  SDL_DestroyTexture(tarknap.kuvat.valittu);
  SDL_DestroyTexture(tarknap.kuvat.ei_valittu);
  tuhoa_slista(&sektus);
  tuhoa_slista(&stulos);
  tuhoa_lista(&ftulos);
  tuhoa_lista(&thetki);
  tuhoa_lista(&jarjes);
  
  tuhoa_slista(&muut_a);
  tuhoa_slista(&muut_b);
  tuhoa_slista(&tietoalut);
  tuhoa_slista(&tietoloput);
  if(lisatd)
    tuhoa_slista(&lisatd);
  
  free(kellool.teksti);
  free(tkstalol.teksti);
 EI_KUVAA:
  TTF_CloseFont(kellool.font);
  TTF_CloseFont(tulosol.font);
  TTF_CloseFont(jarjol1.font);
  TTF_CloseFont(tiedotol.font);
  TTF_CloseFont(sektusol.font);
  TTF_CloseFont(tkstalol.font);
  TTF_CloseFont(tarknap.teksti.font);
  TTF_CloseFont(lisaol.font);
  TTF_CloseFont(muutol.font);
 EI_FONTTI:
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(ikkuna);
  TTF_Quit();
 EI_TTF:
  SDL_Quit();
 EI_SDL:
  return r;
}
