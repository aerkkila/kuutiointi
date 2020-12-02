#include <SDL.h>

char ohjelman_nimi[] = "Kajastin";
int ikkuna_x=0, ikkuna_y=0, ikkuna_w=1600, ikkuna_h=600;
Uint32 viive = 3;
char ulosnimi[] = "tulokset.txt";

char url_valittu[] = "/home/antterkk/kajastin/kuva_valittu.bmp";
char url_eivalittu[] = "/home/antterkk/kajastin/kuva_valittu_ei.bmp";
char tietoalkustr[] = "Avg5|   σ|Avg12|   σ|Keskiarvo|Mediaani";

int kellokoko = 200;
char kellofonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf";
SDL_Rect kellosij = (SDL_Rect){0, 100, 1000, 300};
SDL_Color kellovarit[] = {(SDL_Color){255, 255, 255, 255},	\
			  (SDL_Color){0, 255, 0, 255},		\
			  (SDL_Color){255, 0, 0, 255},		\
			  (SDL_Color){200, 80, 100, 255}};

int tuloskoko = 20;
char tulosfonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf";
SDL_Color tulosvari = {100, 200, 150, 255};
SDL_Rect tulossij = (SDL_Rect){820, 30, 200, 500};

int jarjkoko = 20;
char jarjfonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf";
SDL_Color jarjvari = {170, 100, 110, 255};
SDL_Rect jarjsij = (SDL_Rect){0, 30, 200, 500};
float jarjsuhde = 0.70;

int tiedotkoko = 20;
char tiedotfonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Courier_New.ttf";
SDL_Color tiedotvari = {150, 255, 150, 255};
SDL_Rect tiedotsij = (SDL_Rect){900, 30, 500, 500};

SDL_Rect tluvutsij = (SDL_Rect){900, 30, 500, 500};

int lisakoko = 19;
char lisafonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf";
SDL_Color lisavari = {180, 180, 80, 255};
SDL_Rect lisasij = (SDL_Rect){920, 230, 800, 500};

int sektuskoko = 19;
char sektusfonttied[] = "/usr/share/fonts/truetype/msttcorefonts/Courier_New.ttf";
SDL_Color sektusvari = {255, 255, 255, 255};
SDL_Rect sektussij = (SDL_Rect){0, 390, 1000, 200};

int vntakoko = 12;
char vntateksti[] = "Tarkasteluaika";
char vntafonttied[] = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf";
SDL_Color vntavari = {255, 255, 255, 255};
SDL_Rect vntasij = (SDL_Rect){60, 10, 1000, 90};
