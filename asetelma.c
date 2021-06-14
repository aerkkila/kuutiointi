#include <stdarg.h>
#include <SDL.h>
#include "asetelma.h"

const char ohjelman_nimi[] = "Kajastin";
int ikkuna_x=0, ikkuna_y=0, ikkuna_w=1750, ikkuna_h=600;
Uint32 viive = 3;
const char ulosnimi0[] = "tulokset.txt";
const char uloskansio[] = "/home/antterkk/kuutiointi/";
unsigned NxN = 3;
unsigned karsinta = 16; // N/karsinta+1 parasta ja huonointa tulosta pois keskiarvosta

const char url_valittu[] = "/home/antterkk/kuutiointi/kuva_valittu.bmp";
const char url_eivalittu[] = "/home/antterkk/kuutiointi/kuva_valittu_ei.bmp";
const char tietoalkustr[] = "Avg5|   σ|Avg12|   σ|Keskiarvo|Mediaani";
const char muut_a_str[] = "ulosnimi:|eri_sekunnit|kuvaaja|kuutio|karsintakuvaaja";

tekstiolio_s kellool = {.ttflaji = 1,					\
			.fonttikoko = 200,				\
			.fonttied = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf", \
			.sij = {0, 100, 1000, 300}};

SDL_Color kellovarit[] = {(SDL_Color){255, 255, 255, 255},	\
			  (SDL_Color){0, 255, 0, 255},		\
			  (SDL_Color){255, 0, 0, 255},		\
			  (SDL_Color){200, 80, 100, 255}};

tekstiolio_s tulosol = {.ttflaji = 2,					\
			.sij = {820, 30, 200, 550},			\
			.fonttied = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf", \
			.fonttikoko = 19,				\
			.vari = {100, 200, 150, 255},			\
			.numerointi = 1};

tekstiolio_s jarjol1 = {.ttflaji = 1,					\
			.fonttikoko = 19,				\
			.fonttied = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf", \
			.vari = {140, 150, 170, 255},			\
			.sij = {0, 30, 200, 550}};

tekstiolio_s jarjol2 = {.ttflaji = 1,			\
			.vari = {170, 100, 110, 255}};

float jarjsuhde = 0.70;

/*tähän tulee esim "avg5 = \navg12 = " jne.*/
tekstiolio_s tiedotol = {.ttflaji = 0,					\
			 .fonttikoko = 20,				\
			 .fonttied = "/usr/share/fonts/truetype/msttcorefonts/Courier_New.ttf", \
			 .vari = {150, 255, 150, 255},			\
			 .sij = {900, 30, 500, 500}};

/*tähän tulevat luvut tiedotolion perään */
tekstiolio_s tluvutol;

tekstiolio_s lisaol = {.ttflaji = 2,					\
		       .fonttikoko = 19,				\
		       .fonttied = "/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf", \
		       .vari = {255, 255, 255, 255},			\
		       .sij = {920, 230, 800, 500}};

tekstiolio_s sektusol = {.ttflaji = 0,					\
			 .fonttikoko = 17,				\
			 .fonttied = "/usr/share/fonts/truetype/msttcorefonts/Courier_New.ttf", \
			 .vari = {255, 255, 255, 255},			\
			 .sij = {0, 390, 1500, 200},			\
			 .numerointi = 1};

tekstiolio_s muutol = {.ttflaji = 1,					\
		       .fonttikoko = 14,				\
		       .fonttied = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",	\
		       .vari = {230, 210, 200, 255},			\
		       .sij = {60, 35, 800, 75}};

tekstiolio_s tkstalol = {.ttflaji = 2,					\
			 .fonttikoko = 16,				\
			 .fonttied = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf", \
			 .vari = {230, 230, 230, 255},			\
			 .sij = {75, 10, 800, 75},			\
			 .numerointi = 1};

vnta_s tarknap = {.valittu = 1,						\
		  .teksti = {.ttflaji = 1,				\
			     .fonttikoko = 12,				\
			     .fonttied = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf", \
			     .vari = {255, 255, 255, 255},		\
			     .teksti = "Tarkasteluaika"}};

SDL_Rect tarknapsij = {60, 10, 800, 90};

int avaa_fontit(int n, ...);

int asetelma() {
  if(avaa_fontit(9, &kellool, &tulosol, &jarjol1, &tiedotol, &lisaol, &sektusol, &muutol, &tkstalol, &tarknap.teksti))
    return 1;
  
  kellool.teksti = malloc(90);
  kellool.teksti[0] = 0;
  kellool.vari = kellovarit[0];
  
  jarjol1.sij.h *= jarjsuhde;
  
  jarjol2.sij = jarjol1.sij;
  jarjol2.sij.h *= (1-jarjsuhde);
  jarjol2.sij.y += jarjol1.sij.h;

  tluvutol = tiedotol;

  tkstalol.teksti = malloc(300);
  tkstalol.teksti[0] = 0;

  int rvali = TTF_FontLineSkip(tarknap.teksti.font);
  int d = (int)(rvali*1.2);
  int valipituus;
  TTF_GlyphMetrics(tarknap.teksti.font, ' ', NULL,NULL,NULL,NULL, &valipituus);
  
  tarknap.teksti.sij = (SDL_Rect) {tarknapsij.x + d + 3*valipituus,	\
				   tarknapsij.y + (d - rvali) / 2,	\
				   tarknapsij.w - d - 3*valipituus,	\
				   rvali};
  tarknap.kuvat.sij = (SDL_Rect) {tarknapsij.x, tarknapsij.y, d, d};
  
  return 0;
}

void fonttivirhe(int n) {
  fprintf(stderr, "Virhe: Ei avattu fonttia %i: %s\n", n, TTF_GetError());
}

int avaa_fontti(tekstiolio_s* teol) {
  teol->font = TTF_OpenFont(teol->fonttied, teol->fonttikoko);
  if(!teol->font)
    return 1;
  return 0;
}

int avaa_fontit(int n, ...) {
  va_list ap;
  va_start(ap, n);
  tekstiolio_s* ol;
  for(int i=0; i<n; i++) {
    ol = va_arg(ap, tekstiolio_s*);
    if(avaa_fontti(ol)) {
      fonttivirhe(i);
      return 1;
    }
  }
  va_end(ap);
  return 0;
}

SDL_Renderer* rend;
SDL_Window* ikkuna;
strlista* muut_b;
strlista* muut_a;
strlista* sektus;
strlista* tietoalut;
strlista* tiedot;
strlista* lisatd;
char* ulosnimi;
