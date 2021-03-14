#ifndef __CFG__
#define __CFG__
#include <SDL.h>

extern char* ohjelman_nimi;
extern int ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h;
extern Uint32 viive;
extern char* ulosnimi;
extern char* uloskansio;

extern SDL_Renderer* rend;
extern SDL_Window* ikkuna;

extern tekstiolio_s kellool;
extern tekstiolio_s tulosol;
extern tekstiolio_s jarjol1;
extern tekstiolio_s jarjol2;
extern tekstiolio_s tiedotol;
extern tekstiolio_s tlukuolio;
extern tekstiolio_s lisaol;
extern tekstiolio_s sektusol;
extern tekstiolio_s muutol;
extern tekstiolio_s tkstalol;
extern vnta_s vntaol;

extern char* url_valittu;
extern char* url_eivalittu;
extern char* tietoalkustr;
extern char* muut_a_str;

extern int kellokoko;
extern char* kellofonttied;
extern char kellottflaji;
extern SDL_Rect kellosij;
extern SDL_Color* kellovarit;

extern int tuloskoko;
extern char* tulosfonttied;
extern char tulosttflaji;
extern SDL_Color tulosvari;
extern SDL_Rect tulossij;

extern int jarjkoko;
extern char* jarjfonttied;
extern char jarjttflaji;
extern SDL_Color jarjvari1;
extern SDL_Color jarjvari2;
extern SDL_Rect jarjsij;
extern float jarjsuhde;

extern int tiedotkoko;
extern char* tiedotfonttied;
extern char tiedotttflaji;
extern SDL_Color tiedotvari;
extern SDL_Rect tiedotsij;

extern SDL_Rect tluvutsij;
extern int lisakoko;
extern char* lisafonttied;
extern char lisattflaji;
extern SDL_Color lisavari;
extern SDL_Rect lisasij;

extern int sektuskoko;
extern char* sektusfonttied;
extern char sektusttflaji;
extern SDL_Color sektusvari;
extern SDL_Rect sektussij;

extern int muutkoko;
extern char* muutfonttied;
extern char muutttflaji;
extern SDL_Color muutvari;
extern SDL_Rect muutsij;

extern int tkstalkoko;
extern char* tkstalfonttied;
extern char tkstalttflaji;
extern SDL_Color tkstalvari;
extern SDL_Rect tkstalsij;

extern int vntakoko;
extern char* vntateksti;
extern char* vntafonttied;
extern char vntattflaji;
extern char vntavalittu;
extern SDL_Color vntavari;
extern SDL_Rect vntasij;

#endif
