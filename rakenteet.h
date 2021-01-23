#ifndef __RAKENTEET__
#define __RAKENTEET__

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <listat.h>

typedef struct {
  char* teksti;
  char ttflaji; //mitä ttf-kirjaston funktiota käytetään (katso laita_teksti_ttf())
  TTF_Font* font;
  int fonttikoko;
  char* fonttied;
  SDL_Rect* sij; //tälle varattu maksimitila
  SDL_Rect* toteutuma; //mikä tila oikeasti käytetään
  SDL_Color vari;
  short alku; //koskee vain listoja, ensimmäisen näytetyn indeksi, 'r'
  short rullaus; //koskee vain listoja, 'w'
  char numerointi; //koskee vain listoja;
} tekstiolio_s;

typedef struct {
  SDL_Texture* valittu;
  SDL_Texture* ei_valittu;
  SDL_Rect* sij;
} kuvarakenne;

typedef struct {
  tekstiolio_s* teksti;
  kuvarakenne* kuvat;
  char valittu;
} vnta_s;

typedef struct {
  char kello;
  char valinta;
  char sektus;
  char tulos;
  char jarj;
  char tiedot;
  char lisatd;
  char muut;
  char tkstal;
} laitot_s;

typedef struct {
  strlista* strtulos;
  flista* ftulos;
  ilista* tuloshetki; //unix-aika tuloksen saamishetkellä
  strlista* sijarj;
  flista* fjarj;
  strlista* strjarj;
} tkset_s;

typedef struct {
  SDL_Window* ikkuna;
  SDL_Renderer* rend;
  tekstiolio_s* kello_o;
  tekstiolio_s* tulos_o;
  tekstiolio_s* jarj1_o;
  tekstiolio_s* jarj2_o;
  tekstiolio_s* tiedot_o;
  tekstiolio_s* tluvut_o;
  tekstiolio_s* lisa_o;
  tekstiolio_s* sektus_o;
  tekstiolio_s* muut_o;
  tekstiolio_s* tkstal_o;
  tkset_s* tkset;
  strlista* tiedot;
  strlista* tietoalut;
  strlista* lisatd;
  strlista* sekoitukset;
  laitot_s* laitot;
  strlista* muut_a;
  strlista* muut_b;
  vnta_s* vnta_o;
  Uint32 viive;
  SDL_Color* kvarit;
  char* ulosnimi;
  char* uloskansio;
} kaikki_s;

typedef struct {
  float nyt;
  float min;
  int minind;
  float max;
  int maxind;
} avgtulos;

typedef enum {
  ei = 0,
  plus,
  dnf
} sakko_e;

#endif
