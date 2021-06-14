#ifndef __ASETELMA__
#define __ASETELMA__
#endif

#include <SDL.h>
#include <strlista.h>
#include "grafiikka.h"

extern const char ohjelman_nimi[];
extern int ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h;
extern Uint32 viive;
extern const char ulosnimi0[];
extern char* ulosnimi;
extern const char uloskansio[];
extern unsigned NxN;
extern unsigned karsinta;

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
extern vnta_s tarknap;

extern SDL_Color kellovarit[];

extern strlista* tietoalut;
extern strlista* sektus;
extern strlista* tiedot;
extern strlista* lisatd;
extern strlista* muut_a;
extern strlista* muut_b;

extern const char url_valittu[];
extern const char url_eivalittu[];
extern const char tietoalkustr[];
extern const char muut_a_str[];

int asetelma();
