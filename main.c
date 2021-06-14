#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <listat.h>
#include <time.h>
#include "asetelma.h"
#include "tulokset.h"

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

  asetelma();
  
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
  
  sijarj = _strlisaa_kopioiden(NULL, "");
  fjarj = _flisaa(NULL, -INFINITY);
  strjarj = _strlisaa_kopioiden(NULL, "");

  /*kiinnittämättömät*/
  tietoalut = _yalkuun(_strlistaksi(tietoalkustr, "|"));
  sektus = NULL;
  tiedot = NULL;
  lisatd = NULL;
  muut_a = _yalkuun(_strlistaksi(muut_a_str, "|"));
  muut_b = _strlisaa_kopioiden(muut_b, ulosnimi0);
  ulosnimi = muut_b->str;

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
  _strpoista_kaikki(_yalkuun(tkset.strtulos));
  _yrma(_yalkuun(tkset.ftulos));
  _yrma(_yalkuun(tkset.tuloshetki));
  _yrma(_yalkuun(tkset.fjarj));
  _strpoista_kaikki(_yalkuun(tkset.strjarj));
  _strpoista_kaikki(_yalkuun(tkset.sijarj));
  _strpoista_kaikki(_yalkuun(tietoalut));
  _strpoista_kaikki(_yalkuun(sektus));
  _strpoista_kaikki(_yalkuun(tiedot));
  _strpoista_kaikki(_yalkuun(muut_a));
  _strpoista_kaikki(_yalkuun(muut_b));
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
