/*Make-komento kopioi asetelma.c:n tiedostoksi asetelma1.c ja korvaa MAKE_LIITÄ_*-ilmaisut sopivilla asioilla.
  Muut kuin väliaikaiset muokkaukset pitää tehdä asetelma.c:hen eikä asetelma1.c:hen*/

#include <stdarg.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "asetelma.h"

int ikkuna_x=0, ikkuna_y=0, ikkuna_w=1600, ikkuna_h=600;
Uint32 viive = 3;
unsigned NxN = 3;
unsigned karsinta = 16; // N/karsinta+1 parasta ja huonointa tulosta pois keskiarvosta
float jarjsuhde = 0.70;

int aaniraja = 12; //montako äänireunaa vaaditaan signaalin hyväksyntään
double aanikesto = 0.25; //sekunteina minkä ajan kuluessa äänireunat pitää saada
double aani_turvavali = 3.0; //aika, jonka sisällä äänellä lopetuksesta ääni ei voi käynnistää uutta ajanottoa

const char* restrict ohjelman_nimi = "Skello";
const char* restrict ulosnimi0 = "tulokset.txt";
const char* restrict url_valittu = KOTIKANSIO "/valittu.bmp";
const char* restrict url_eivalittu = KOTIKANSIO "/eivalittu.bmp";
const char* restrict tietoalkustr = "Avg5|   σ|Avg12|   σ|Keskiarvo|Mediaani";
const char* restrict muut_a_str = "ulosnimi: |eri_sekunnit|kuvaaja|kuutio|ääni: pois|autokuutio";

SDL_Color kohdistinvari = {255,255,255,255};
SDL_Color taustavari = {0,0,0,255};

#define MONOFONTTI "MAKE_LIITÄ_MONOFONTTI"
#define YLEISFONTTI "MAKE_LIITÄ_SANSFONTTI"

tekstiolio_s kellool = {.ttflaji = 1,					\
			.fonttikoko = 200,				\
			.fonttied = YLEISFONTTI,			\
			.sij = {0, 50, 2500, 4000}};

SDL_Color kellovarit[] = {(SDL_Color){255, 255, 255, 255},	\
			  (SDL_Color){0,   255, 0,   255},	\
			  (SDL_Color){255, 0,   0,   255},	\
			  (SDL_Color){200, 80,  100, 255}}; 

tekstiolio_s tulosol = {.ttflaji = 1,					\
			.sij = {770, 20, 200, 550},			\
			.fonttied = YLEISFONTTI, \
			.fonttikoko = 21,				\
			.vari = {100, 200, 150, 255},			\
			.numerointi = 1};

tekstiolio_s jarjol1 = {.ttflaji = 1,					\
			.fonttikoko = 21,				\
			.fonttied = YLEISFONTTI,			\
			.vari = {140, 150, 170, 255},			\
			.sij = {.w=300}};

tekstiolio_s jarjol2 = {.vari = {170, 100, 110, 255}};

/*tähän tulee esim "avg5 = \navg12 = " jne.*/
tekstiolio_s tiedotol = {.ttflaji = 1,					\
			 .fonttikoko = 20,				\
			 .fonttied = MONOFONTTI, \
			 .vari = {150, 255, 150, 255},			\
			 .sij = {900, 20, 500, 500}};

/*tähän tulevat luvut tiedotolion perään */
tekstiolio_s tluvutol;

tekstiolio_s lisaol = {.ttflaji = 2,					\
		       .fonttikoko = 19,				\
		       .fonttied = YLEISFONTTI, \
		       .vari = {255, 255, 255, 255},			\
		       .sij = {920, 230, 800, 500}};

tekstiolio_s sektusol = {.ttflaji = 1,					\
			 .fonttikoko = 20,				\
			 .fonttied = MONOFONTTI,			\
			 .vari = {255, 255, 255, 255},			\
			 .sij = {0, 390, 1500, 200},			\
			 .numerointi = 1};

tekstiolio_s muutol = {.ttflaji = 1,					\
		       .fonttikoko = 12,				\
		       .fonttied = MONOFONTTI,				\
		       .vari = {230, 210, 200, 255},			\
		       .sij = {60, 35, 800, 75}};

tekstiolio_s tkstalol = {.ttflaji = 2,					\
			 .fonttikoko = 16,				\
			 .fonttied = MONOFONTTI,			\
			 .vari = {230, 230, 230, 255},			\
			 .sij = {75, 10, 800, 75},			\
			 .numerointi = 1};

vnta_s tarknap = {.valittu = 1,						\
		  .teksti = {.ttflaji = 1,				\
			     .fonttikoko = 12,				\
			     .fonttied = MONOFONTTI,			\
			     .vari = {255, 255, 255, 255},		\
			     .teksti = "Tarkasteluaika"}};

korostustietue korostusol = {
  .vari = {30, 120, 255, 255},
  .paksuus = -3, //kerrotaan -1:llä, jos otetaan käyttöön
};

SDL_Rect tarknapsij = {60, 10, 800, 90};

slista* muut_a;
slista* tietoalut;
slista* tietoloput;
slista* lisatd;

slista* sektus;
slista* stulos;
flista* ftulos;
ilista* thetki;
int* jarjes;
float* fjarje;
char* ulosnimi;

int avaa_fontit(int n, ...);

#define tulospatka 10
int asetelma() {
  muut_a     = slistaksi(muut_a_str, "|");
  tietoalut  = slistaksi(tietoalkustr, "|");
  tietoloput = alusta_lista(tietoalut->pit, char*);
  lisatd     = alusta_lista(13, char*);
  
  sektus = alusta_lista(tulospatka, char*);
  stulos = alusta_lista(tulospatka, char*);
  ftulos = alusta_lista(tulospatka, float);
  thetki = alusta_lista(tulospatka, int);
  jarjes = malloc(1);
  fjarje = malloc(1);

  int pit0 = strlen(muut_a->taul[0]); //ulosnimi
  char tmpc[500];
  sprintf(tmpc, "%s/%s/%s", getenv("HOME"), TULOSKANSIO, ulosnimi0);
  muut_a->taul[0] = realloc(muut_a->taul[0], pit0+strlen(tmpc)+1);
  strcat(muut_a->taul[0], tmpc);
  ulosnimi = muut_a->taul[0] + pit0;
  
  if(avaa_fontit(9, &kellool, &tulosol, &jarjol1, &tiedotol, &lisaol, &sektusol, &muutol, &tkstalol, &tarknap.teksti))
    return 1;
  
  kellool.teksti = malloc(90);
  kellool.teksti[0] = 0;
  kellool.vari = kellovarit[0];

  SDL_Color vtmp = jarjol2.vari;
  jarjol1.teksti = malloc(32);
  jarjol2 = jarjol1;
  jarjol2.vari = vtmp;

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

void fonttivirhe(int n, const char* mjon) {
  fprintf(stderr, "Virhe: Ei avattu fonttia %i\n%s\n: %s\n", n, mjon, TTF_GetError());
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
      fonttivirhe(i, ol->fonttied);
      return 1;
    }
  }
  va_end(ap);
  return 0;
}

void vakiosijainnit() {
  unsigned laitot0 = laitot;
  laitot = jaaduta;
  piirra();
  kellool.sij.y = muutol.toteutuma.y + muutol.toteutuma.h;
  kellool.sij.w = ikkuna_w - kellool.toteutuma.x;
  piirra(); //toistetaan varalta
  sektusol.sij.y = kellool.toteutuma.y + kellool.toteutuma.h + 5;
  piirra();
  laitot = laitot0;
}

void tuhoa_asetelma() {
  SDL_DestroyTexture(tarknap.kuvat.valittu);
  SDL_DestroyTexture(tarknap.kuvat.ei_valittu);
  tuhoa_slista(&sektus);
  tuhoa_slista(&stulos);
  tuhoa_lista(&ftulos);
  tuhoa_lista(&thetki);
  free(fjarje); fjarje=NULL;
  free(jarjes); jarjes=NULL;
  tuhoa_slista(&muut_a);
  tuhoa_slista(&tietoalut);
  tuhoa_slista(&tietoloput);
  if(lisatd)
    tuhoa_slista(&lisatd);
  
  free(kellool.teksti);
  free(tkstalol.teksti);
  free(jarjol1.teksti);
  TTF_CloseFont(kellool.font);
  TTF_CloseFont(tulosol.font);
  TTF_CloseFont(jarjol1.font);
  TTF_CloseFont(tiedotol.font);
  TTF_CloseFont(sektusol.font);
  TTF_CloseFont(tkstalol.font);
  TTF_CloseFont(tarknap.teksti.font);
  TTF_CloseFont(lisaol.font);
  TTF_CloseFont(muutol.font);
}

SDL_Renderer* rend;
SDL_Window* ikkuna;
SDL_Texture* tausta;
