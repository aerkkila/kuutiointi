#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <listat.h>
#include <time.h>
#include "cfg.h"
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

  /*kello-olio*/
  kellool.teksti = malloc(90);
  strcpy(kellool.teksti, "");
  kellool.ttflaji = kellottflaji;
  kellool.font = TTF_OpenFont(kellofonttied, kellokoko);
  kellool.fonttikoko = kellokoko;
  kellool.fonttied = kellofonttied;
  if(!kellool.font) {
    fprintf(stderr, "Virhe: Ei avattu kellofonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  kellool.sij = &kellosij;
  SDL_Rect kelloapu = (SDL_Rect){0, 0, 0, 0};
  kellool.toteutuma = &kelloapu;
  kellool.vari = kellovarit[0];

  /*tulosolio*/
  tulosol.ttflaji = tulosttflaji;
  tulosol.font = TTF_OpenFont(tulosfonttied, tuloskoko);
  tulosol.fonttikoko = tuloskoko;
  tulosol.fonttied = tulosfonttied;
  if(!tulosol.font) {
    fprintf(stderr, "Virhe: Ei avattu tulosfonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  tulosol.sij = &tulossij;
  SDL_Rect tulosapu = (SDL_Rect){0, 0, 0, 0};
  tulosol.toteutuma = &tulosapu;
  tulosol.vari = tulosvari;
  tulosol.rullaus = 0;
  tulosol.numerointi = 1;

  /*jarjolio1*/
  jarjol1.ttflaji = jarjttflaji;
  jarjol1.font = TTF_OpenFont(jarjfonttied, jarjkoko);
  jarjol1.fonttikoko = jarjkoko;
  jarjol1.fonttied = jarjfonttied;
  if(!jarjol1.font) {
    fprintf(stderr, "Virhe: Ei avattu äärifonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  SDL_Rect jarjsij1 = jarjsij;
  jarjol1.sij = &jarjsij1;
  jarjol1.sij->h *= jarjsuhde;
  SDL_Rect jarjapu1 = (SDL_Rect){0, 0, 0, 0};
  jarjol1.toteutuma = &jarjapu1;
  jarjol1.vari = jarjvari1;
  jarjol1.rullaus = 0;
  jarjol1.numerointi = 0;

  /*jarjolio2*/
  jarjol2.ttflaji = jarjttflaji;
  jarjol2.font = jarjol1.font;
  SDL_Rect jarjsij2 = jarjsij;
  jarjol2.sij = &jarjsij2;
  jarjol2.sij->h *= (1-jarjsuhde);
  jarjol2.sij->y += jarjol1.sij->h;
  SDL_Rect jarjapu2 = (SDL_Rect){0, 0, 0, 0};
  jarjol2.toteutuma = &jarjapu2;
  jarjol2.vari = jarjvari2;
  jarjol2.rullaus = 0;
  jarjol2.numerointi = 0;

  /*tiedotolio*/
  tiedotol.ttflaji = tiedotttflaji;
  tiedotol.font = TTF_OpenFont(tiedotfonttied, tiedotkoko);
  tiedotol.fonttikoko = tiedotkoko;
  if(!tiedotol.font) {
    fprintf(stderr, "Virhe: Ei avattu tiedotfonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  tiedotol.sij = &tiedotsij;
  SDL_Rect tiedotapu = (SDL_Rect){0, 0, 0, 0};
  tiedotol.toteutuma = &tiedotapu;
  tiedotol.vari = tiedotvari;
  tiedotol.rullaus = 0;
  tiedotol.numerointi = 0;

  /*tluvutolio on pitkälti sama kuin tiedotolio*/
  tluvutol.ttflaji = tiedotttflaji;
  tluvutol.font = tiedotol.font;
  tluvutol.sij = &tluvutsij;
  SDL_Rect tlukuapu = (SDL_Rect){0, 0, 0, 0};
  tluvutol.toteutuma = &tlukuapu;
  tluvutol.vari = tiedotvari;
  tluvutol.rullaus = 0;
  tluvutol.numerointi = 0;

  /*lisätiedot*/
  lisaol.ttflaji = lisattflaji;
  lisaol.font = TTF_OpenFont(lisafonttied, lisakoko);
  lisaol.fonttikoko = lisakoko;
  lisaol.fonttied = lisafonttied;
  if(!lisaol.font) {
    fprintf(stderr, "Virhe: Ei avattu lisafonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  lisaol.sij = &lisasij;
  SDL_Rect lisaapu = (SDL_Rect){0, 0, 0, 0};
  lisaol.toteutuma = &lisaapu;
  lisaol.vari = lisavari;
  lisaol.rullaus = 0;
  lisaol.numerointi = 0;

  /*sekoitusolio*/
  sektusol.ttflaji = sektusttflaji;
  sektusol.font = TTF_OpenFont(sektusfonttied, sektuskoko);
  sektusol.fonttikoko = sektuskoko;
  sektusol.fonttied = sektusfonttied;
  if(!sektusol.font) {
    fprintf(stderr, "Virhe: Ei avattu sekoitusfonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  sektusol.sij = &sektussij;
  SDL_Rect sektusapu = (SDL_Rect){0, 0, 0, 0};
  sektusol.toteutuma = &sektusapu;
  sektusol.vari = sektusvari;
  sektusol.rullaus = 0;
  sektusol.numerointi = 1;

  /*muutolio*/
  muutol.ttflaji = muutttflaji;
  muutol.font = TTF_OpenFont(muutfonttied, muutkoko);
  muutol.fonttikoko = muutkoko;
  if(!muutol.font) {
    fprintf(stderr, "Virhe: Ei avattu muutfonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  muutol.sij = &muutsij;
  SDL_Rect muut_toteutuma = (SDL_Rect){0, 0, 0, 0};
  muutol.toteutuma = &muut_toteutuma;
  muutol.vari = muutvari;
  muutol.rullaus = 0;
  muutol.numerointi = 0;

  /*tekstialueolio*/
  tkstalol.teksti = calloc(300, 1);
  tkstalol.ttflaji = tkstalttflaji;
  tkstalol.font = TTF_OpenFont(tkstalfonttied, tkstalkoko);
  tkstalol.fonttikoko = tkstalkoko;
  if(!tkstalol.font) {
    fprintf(stderr, "Virhe: Ei avattu sekoitusfonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  tkstalol.sij = &tkstalsij;
  SDL_Rect tkstalapu = (SDL_Rect){0, 0, 0, 0};
  tkstalol.toteutuma = &tkstalapu;
  tkstalol.vari = tkstalvari;
  tkstalol.rullaus = 0;
  tkstalol.numerointi = 1;

  /*valintaolion teksti*/
  vntaol.valittu = vntavalittu;
  tekstiolio_s vto;
  vto.ttflaji = 2;
  vto.font = TTF_OpenFont(vntafonttied, vntakoko);
  vto.fonttikoko = vntakoko;
  if(!vto.font) {
    fprintf(stderr, "Virhe: Ei avattu valintafonttia: %s\n", TTF_GetError());
    r = 1;
    goto EI_FONTTI;
  }
  int rvali = TTF_FontLineSkip(vto.font);
  int d = (int)(rvali*1.2);
  int valipituus;
  TTF_GlyphMetrics(vto.font,' ',NULL,NULL,NULL,NULL,&valipituus);
  SDL_Rect ta = (SDL_Rect){vntasij.x + d + 3*valipituus,	\
			   vntasij.y + (d - rvali) / 2,	\
			   vntasij.w - d - 3*valipituus,	\
			   rvali};
  SDL_Rect vntaapu = (SDL_Rect){0, 0, 0, 0};
  vto.sij = &ta;
  vto.teksti = vntateksti;
  vto.toteutuma = &vntaapu;
  vto.vari = vntavari;
  vto.rullaus = 0;
  vto.numerointi = 0;
  vntaol.teksti = vto;
  
  /*valintaolion kuvat*/
  kuvarakenne kuvat;
  SDL_Surface* kuva;
  if(!(kuva = SDL_LoadBMP(url_valittu))) {
    fprintf(stderr, "Virhe: Ei luettu kuvaa \"%s\"\n", url_valittu);
    goto EI_KUVAA;
  }
  kuvat.valittu = SDL_CreateTextureFromSurface(rend, kuva);
  SDL_FreeSurface(kuva);
  if(!(kuva = SDL_LoadBMP(url_eivalittu))) {
    fprintf(stderr, "Virhe: Ei luettu kuvaa \"%s\"\n", url_eivalittu);
    goto EI_KUVAA;
  }
  kuvat.ei_valittu = SDL_CreateTextureFromSurface(rend, kuva);
  SDL_FreeSurface(kuva);
  SDL_Rect kuvaalue;
  kuvaalue.x = vntasij.x;
  kuvaalue.y = vntasij.y;
  kuvaalue.w = d;
  kuvaalue.h = d;
  kuvat.sij = &kuvaalue;
  vntaol.kuvat = &kuvat;
  
  tkset.strtulos = NULL;
  tkset.ftulos = NULL;
  tkset.tuloshetki = NULL;
  tkset.sijarj = _strlisaa_kopioiden(NULL, "");
  tkset.fjarj = _flisaa(NULL, -INFINITY);
  tkset.strjarj = _strlisaa_kopioiden(NULL, "");

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

  SDL_DestroyTexture(vntaol.kuvat->valittu);
  SDL_DestroyTexture(vntaol.kuvat->ei_valittu);
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
  TTF_CloseFont(vto.font);
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
