#ifndef __ASETELMA__
#define __ASETELMA__

#include <SDL2/SDL.h>
#include "listat.h"
#include "grafiikka.h"

#define TULOSKANSIO "skello"
#define KOTIKANSIO "/usr/share/skello"

extern int ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h;
extern Uint32 viive;
extern unsigned NxN;
extern unsigned karsinta;
extern float jarjsuhde;

extern int aaniraja;
extern double aanikesto;
extern double aani_turvavali;

extern const char* restrict ohjelman_nimi;
extern const char* restrict ulosnimi0;
extern const char* restrict url_valittu;
extern const char* restrict url_eivalittu;
extern const char* restrict tietoalkustr;
extern const char* restrict muut_a_str;
extern char* ulosnimi;
extern SDL_Color kohdistinvari;
extern SDL_Color taustavari;

extern SDL_Renderer* rend;
extern SDL_Window* ikkuna;
extern SDL_Texture* tausta;

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
extern vnta_s tarknap;

extern SDL_Color kellovarit[];

extern slista* muut_a;
extern slista* tietoalut;
extern slista* tietoloput;
extern slista* lisatd;

extern slista* sektus;
extern slista* stulos;
extern flista* ftulos;
extern ilista* thetki;
extern int* jarjes;
extern float* fjarje;

int asetelma();
void vakiosijainnit();
void tuhoa_asetelma();

#endif
