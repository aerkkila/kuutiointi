#include <SDL.h>
#include "cfg.h"

const char ohjelman_nimi[] = "Kajastin";
int ikkuna_x=0, ikkuna_y=0, ikkuna_w=1750, ikkuna_h=600;
Uint32 viive = 3;
const char ulosnimi0[] = "tulokset.txt";
const char uloskansio[] = "/home/antterkk/kajastin/";
unsigned char NxN = 3;

char url_valittu[] = "/home/antterkk/kajastin/kuva_valittu.bmp";
char url_eivalittu[] = "/home/antterkk/kajastin/kuva_valittu_ei.bmp";
char tietoalkustr[] = "Avg5|   σ|Avg12|   σ|Keskiarvo|Mediaani";
char muut_a_str[] = "ulosnimi:|eri_sekunnit|kuvaaja|kuutio|nauhoituslaitteet|ääniajurit";

int kellokoko = 200;
char kellofonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf";
char kellottflaji = 1;
SDL_Rect kellosij = (SDL_Rect){0, 100, 1000, 300};
SDL_Color kellovarit[] = {(SDL_Color){255, 255, 255, 255},	\
			  (SDL_Color){0, 255, 0, 255},		\
			  (SDL_Color){255, 0, 0, 255},		\
			  (SDL_Color){200, 80, 100, 255}};

int tuloskoko = 19;
char tulosfonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf";
char tulosttflaji = 2;
SDL_Color tulosvari = {100, 200, 150, 255};
SDL_Rect tulossij = (SDL_Rect){820, 30, 200, 550};

int jarjkoko = 19;
char jarjfonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf";
char jarjttflaji = 2;
SDL_Color jarjvari1 = {140, 150, 170, 255};
SDL_Color jarjvari2 = {170, 100, 110, 255};
SDL_Rect jarjsij = (SDL_Rect){0, 30, 200, 550};
float jarjsuhde = 0.70;

int tiedotkoko = 20;
char tiedotfonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Courier_New.ttf";
char tiedotttflaji = 0;
SDL_Color tiedotvari = {150, 255, 150, 255};
SDL_Rect tiedotsij = (SDL_Rect){900, 30, 500, 500};

SDL_Rect tluvutsij = (SDL_Rect){900, 30, 500, 500};

int lisakoko = 19;
char lisafonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf";
char lisattflaji = 2;
SDL_Color lisavari = {255, 255, 255, 255};
SDL_Rect lisasij = (SDL_Rect){920, 230, 800, 500};

int sektuskoko = 14;
char sektusfonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Courier_New.ttf";
char sektusttflaji = 0;
SDL_Color sektusvari = {255, 255, 255, 255};
SDL_Rect sektussij = (SDL_Rect){0, 390, 1500, 200};

int muutkoko = 14;
char muutfonttied[] = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf";
char muutttflaji = 2;
SDL_Color muutvari = {230, 210, 200};
SDL_Rect muutsij = {60, 35, 800, 75};

int tkstalkoko = 16;
char tkstalfonttied[] = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf";
char tkstalttflaji = 2;
SDL_Color tkstalvari = {230, 230, 230};
SDL_Rect tkstalsij = {75, 10, 800, 75};

int vntakoko = 12;
char vntateksti[] = "Tarkasteluaika";
char vntafonttied[] = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf";
char vntattflaji = 2;
char vntavalittu = 1;
SDL_Color vntavari = {255, 255, 255, 255};
SDL_Rect vntasij = (SDL_Rect){60, 10, 800, 90};

/*nämä asetetaan pääfunktiossa tai myöhemmin*/
tekstiolio_s kellool;
tekstiolio_s tulosol;
tekstiolio_s jarjol1;
tekstiolio_s jarjol2;
tekstiolio_s tiedotol;
tekstiolio_s tluvutol;
tekstiolio_s lisaol;
tekstiolio_s sektusol;
tekstiolio_s muutol;
tekstiolio_s tkstalol;
vnta_s vntaol;
tkset_s tkset;
SDL_Renderer* rend;
SDL_Window* ikkuna;
strlista* muut_b;
strlista* muut_a;
strlista* sektus;
strlista* tietoalut;
strlista* tiedot;
strlista* lisatd;
char* ulosnimi;
