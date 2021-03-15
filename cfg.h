#ifndef __CFG__
#define __CFG__
#include <SDL.h>
#include <SDL_ttf.h>
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
  tekstiolio_s teksti;
  kuvarakenne* kuvat;
  char valittu;
} vnta_s;

typedef struct {
  float nyt;
  float min;
  int minind;
  float max;
  int maxind;
} avgtulos;

typedef struct {
  strlista* strtulos;
  flista* ftulos;
  ilista* tuloshetki; //unix-aika tuloksen saamishetkellä
  strlista* sijarj;
  flista* fjarj;
  strlista* strjarj;
} tkset_s;

typedef enum {
  ei = 0,
  plus,
  dnf
} sakko_e;

#endif

extern const char ohjelman_nimi[];
extern int ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h;
extern Uint32 viive;
extern const char ulosnimi0[];
extern char* ulosnimi;
extern const char uloskansio[];
extern unsigned char NxN;

extern SDL_Renderer* rend;
extern SDL_Window* ikkuna;

extern tekstiolio_s kellool;
extern tekstiolio_s tulosol;
extern tekstiolio_s jarjol1;
extern tekstiolio_s jarjol2;
extern tekstiolio_s tiedotol;
extern tekstiolio_s tluvutol;
extern tekstiolio_s lisaol;
extern tekstiolio_s sektusol;
extern tekstiolio_s muutol;
extern tekstiolio_s tkstalol;
extern tkset_s tkset;
extern vnta_s vntaol;

extern strlista* tietoalut;
extern strlista* sektus;
extern strlista* tiedot;
extern strlista* lisatd;
extern strlista* muut_a;
extern strlista* muut_b;

extern char url_valittu[];
extern char url_eivalittu[];
extern char tietoalkustr[];
extern char muut_a_str[];

extern int kellokoko;
extern char kellofonttied[];
extern char kellottflaji;
extern SDL_Rect kellosij;
extern SDL_Color kellovarit[];

extern int tuloskoko;
extern char tulosfonttied[];
extern char tulosttflaji;
extern SDL_Color tulosvari;
extern SDL_Rect tulossij;

extern int jarjkoko;
extern char jarjfonttied[];
extern char jarjttflaji;
extern SDL_Color jarjvari1;
extern SDL_Color jarjvari2;
extern SDL_Rect jarjsij;
extern float jarjsuhde;

extern int tiedotkoko;
extern char tiedotfonttied[];
extern char tiedotttflaji;
extern SDL_Color tiedotvari;
extern SDL_Rect tiedotsij;

extern SDL_Rect tluvutsij;
extern int lisakoko;
extern char lisafonttied[];
extern char lisattflaji;
extern SDL_Color lisavari;
extern SDL_Rect lisasij;

extern int sektuskoko;
extern char sektusfonttied[];
extern char sektusttflaji;
extern SDL_Color sektusvari;
extern SDL_Rect sektussij;

extern int muutkoko;
extern char muutfonttied[];
extern char muutttflaji;
extern SDL_Color muutvari;
extern SDL_Rect muutsij;

extern int tkstalkoko;
extern char tkstalfonttied[];
extern char tkstalttflaji;
extern SDL_Color tkstalvari;
extern SDL_Rect tkstalsij;

extern int vntakoko;
extern char vntateksti[];
extern char vntafonttied[];
extern char vntattflaji;
extern char vntavalittu;
extern SDL_Color vntavari;
extern SDL_Rect vntasij;
