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
int sivutilaa = 16;

int aaniraja = 12; //montako äänireunaa vaaditaan signaalin hyväksyntään
double aanikesto = 0.25; //sekunteina minkä ajan kuluessa äänireunat pitää saada
double aani_turvavali = 3.0; //aika, jonka sisällä äänellä lopetuksesta ääni ei voi käynnistää uutta ajanottoa

const char* restrict ohjelman_nimi = "Skello";
const char* restrict ulosnimi0 = "tulokset.txt";
const char* restrict url_valittu = KOTIKANSIO "/valittu.bmp";
const char* restrict url_eivalittu = KOTIKANSIO "/eivalittu.bmp";
const char* restrict tietoalkustr = "Avg5|   σ|Avg12|   σ|Keskiarvo|Mediaani";

/* Seuraavissa järjestys on numeroitu asetelma.h:ssa. Järjestyksen muuttaminen pitää tehdä myös siellä */
const char* restrict valikko_str = "tarkasteluaika: |ulosnimi: |eri_sekunnit|kuvaaja|kuutio|ääni: |autokuutio";
const char* aanivaihtoehdot[] = {"pois", "kuuntelu", "pysäytys"};

const char* tarkastelu_str[] = {"pois", "päällä"};
int tarkasteluaikatila = 1;
enum aanivaihtoehto äänitila = aani_pois_e;
char* äänitila_str;

const char* tekstialue[] = {
    "Ajan syöttö",
    "Ulosnimen vaihto",
    "Tuloslistan alkukohta",
    "Avattava tiedosto",
    "Kuution koko (NxNxN)",
    "Keskiarvon karsinta",
};
SDL_Color kohdistinvari = {255,255,255,255};
SDL_Color taustavari = {0,0,0,255};

#define MONOFONTTI "MAKE_LIITÄ_MONOFONTTI"
#define YLEISFONTTI "MAKE_LIITÄ_SANSFONTTI"

enum oliot oliojärjestys[] = {
    valikko_oe, sektus_oe, kello_oe, Null_oe,
    tulos_oe, Null_oe,
    jarj_oe, Null_oe, // Tämän alle ei voi laittaa. jarjol1.alareuna ≠ jarjol2.alareuna
    tilastot_oe, lisä_oe, Null_oe,
};

tekstiolio_s kellool = {
    .ttflaji = 1,
    .fonttikoko = 200,
    .fonttied = YLEISFONTTI,
};

SDL_Color kellovärit[] = {
    (SDL_Color){255, 255, 255, 255},
    (SDL_Color){0,   255, 0,   255},
    (SDL_Color){255, 0,   0,   255},
    (SDL_Color){200, 80,  100, 255}
};

tekstiolio_s tulosol = {
    .ttflaji = 1,
    .fonttied = YLEISFONTTI,
    .fonttikoko = 21,
    .vari = {100, 200, 150, 255},
    .numerointi = 1
};

tekstiolio_s jarjol1 = {
    .ttflaji = 1,
    .fonttikoko = 21,
    .fonttied = YLEISFONTTI,
    .vari = {140, 150, 170, 255},
};
tekstiolio_s jarjol2 = {.vari = {170, 100, 110, 255}};

/* tähän tulee esim "avg5 = \navg12 = " jne. */
tekstiolio_s tilastotol = {
    .ttflaji = 1,
    .fonttikoko = 20,
    .fonttied = MONOFONTTI,
    .vari = {150, 255, 150, 255}
};
tekstiolio_s tilastoluvutol; // tämän parametreja ei tarvitse asettaa

tekstiolio_s lisaol = {
    .ttflaji = 2,
    .fonttikoko = 19,
    .fonttied = YLEISFONTTI,
    .vari = {255, 255, 255, 255}
};

tekstiolio_s sektusol = {
    .ttflaji = 1,
    .fonttikoko = 20,
    .fonttied = MONOFONTTI,
    .monirivinen = 1,
    .vari = {255, 255, 255, 255},
    .numerointi = 1
};

tekstiolio_s valikkool = {
    .ttflaji = 1,
    .fonttikoko = 12,
    .fonttied = MONOFONTTI,
    .vari = {255, 255, 255, 255},
    .y = 8,
    .x = 16,
};

tekstiolio_s tkstalol = {
    .ttflaji = 1,
    .fonttikoko = 16,
    .fonttied = MONOFONTTI,
    .vari = {255, 255, 255, 255},
    .numerointi = 1
};

tekstiolio_s *tkstoliot[] = {
    [kello_oe]=&kellool, [sektus_oe]=&sektusol, [tulos_oe]=&tulosol,
    [jarj_oe]=&jarjol1, [tilastot_oe]=&tilastotol,
    [lisä_oe]=&lisaol, [valikko_oe]=&valikkool, [tkstal_oe]=&tkstalol,
};

korostustietue korostusol = {
    .vari = {30, 120, 255, 255},
    .paksuus = -3, //kerrotaan -1:llä, jos otetaan käyttöön
};

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

int avaa_fontit(tekstiolio_s*, ...);

#define tulospatka 10
int asetelma() {
    muut_a     = slistaksi(valikko_str, "|");
    tietoalut  = slistaksi(tietoalkustr, "|");
    tietoloput = alusta_lista(tietoalut->pit, char*);
    lisatd     = alusta_lista(13, char*);
  
    sektus = alusta_lista(tulospatka, char*);
    stulos = alusta_lista(tulospatka, char*);
    ftulos = alusta_lista(tulospatka, float);
    thetki = alusta_lista(tulospatka, int);

#define apuc (muut_a->taul[tarkasteluaika_e])
    int pit0 = strlen(apuc);
    apuc = realloc(apuc, pit0+32);
    strcpy(apuc+pit0, tarkastelu_str[tarkasteluaikatila]);
#undef apuc

    pit0 = strlen(muut_a->taul[ulosnimi_e]);
    char tmpc[500];
    sprintf(tmpc, "%s/%s/%s", getenv("HOME"), TULOSKANSIO, ulosnimi0);
    muut_a->taul[ulosnimi_e] = realloc(muut_a->taul[ulosnimi_e], pit0+strlen(tmpc)+1);
    strcat(muut_a->taul[ulosnimi_e], tmpc);
    ulosnimi = muut_a->taul[ulosnimi_e] + pit0;

    pit0 = strlen(muut_a->taul[aani_e]);
    int pit1 = 0;
    for(int i=0; i<sizeof(aanivaihtoehdot)/sizeof(aanivaihtoehdot[0]); i++)
	if(strlen(aanivaihtoehdot[i]) > pit1)
	    pit1 = strlen(aanivaihtoehdot[i]);
    muut_a->taul[aani_e] = realloc(muut_a->taul[aani_e], pit0+pit1+1);
    äänitila_str = muut_a->taul[aani_e] + pit0;
    strcpy(äänitila_str, aanivaihtoehdot[äänitila]);
  
    if(avaa_fontit(&kellool, &tulosol, &jarjol1, &tilastotol, &lisaol, &sektusol, &valikkool, &tkstalol, NULL))
	return 1;
  
    kellool.teksti = malloc(90);
    kellool.teksti[0] = 0;
    kellool.vari = kellovärit[0];

    SDL_Color vtmp = jarjol2.vari;
    jarjol1.teksti = malloc(32);
    jarjol2 = jarjol1;
    jarjol2.vari = vtmp;

    tilastoluvutol = tilastotol;

    tkstalol.teksti = malloc(300);
    tkstalol.teksti[0] = 0;
    return 0;
}

void fonttivirhe(const char* mjon) {
    fprintf(stderr, "Ei avattu fonttia %s\n: %s\n", mjon, TTF_GetError());
}

int avaa_fontti(tekstiolio_s* teol) {
    teol->font = TTF_OpenFont(teol->fonttied, teol->fonttikoko);
    if(!teol->font)
	return 1;
    return 0;
}

int avaa_fontit(tekstiolio_s *ol, ...) {
    va_list ap;
    va_start(ap, ol);
    do
	if(avaa_fontti(ol)) {
	    fonttivirhe(ol->fonttied);
	    return 1;
	}
    while((ol=va_arg(ap, tekstiolio_s*)));
    va_end(ap);
    return 0;
}

unsigned vakiosijainnit() {
    unsigned laitot0=laitot, laitetut=0;
    int ind=0, iii=oliojärjestys[0];
    int pit = sizeof(oliojärjestys)/sizeof(*oliojärjestys);
    SDL_Rect *tot, *sij, *tot_;
    int x=0, xoff, xoff_;
    while(1) {
	/* Varataan kaikille vähintään yksi rivi tilaa. */
	int ind_=ind, iii_=iii, h[32], spit=0;
	while(1) {
	    h[spit] = TTF_FontLineSkip(tkstoliot[iii_]->font);
	    if(++spit==32) break;
	    if(++ind_==pit || (iii_=oliojärjestys[ind_])==Null_oe) goto valmis;
	}
	fprintf(stderr, "Rivin loppua ei löytynyt funktiossa vakiosijainnit\n");
    valmis:
	for(int i=spit-2; i>=0; i--)
	    h[i] += h[i+1];
	memmove(h, h+1, (spit-1)*sizeof(int));
	h[spit-1] = 0;

	/* sarakken ensimmäinen */
	laitetut |= (laitot = 1<<iii);
	xoff = tkstoliot[iii]->x;
	sij = &tkstoliot[iii]->sij;
	sij->x = x + xoff;
	sij->y = tkstoliot[iii]->y;
	sij->h = ikkuna_h - h[0] - sij->y;
	piirrä(0);
	tot_ = &tkstoliot[iii]->toteutuma;
	int maxw = tkstoliot[iii]->toteutuma.w + xoff;

	/* sarakkeen loput */
	iii=oliojärjestys[++ind];
	for(int i=1; i<spit; i++, iii=oliojärjestys[++ind]) {
	    laitetut |= (laitot = 1<<iii);
	    xoff_ = xoff;
	    xoff = tkstoliot[iii]->x;
	    sij = &tkstoliot[iii]->sij;
	    sij->x = tot_->x - xoff_ + xoff;
	    sij->y = tot_->y + tot_->h + tkstoliot[iii]->y;
	    sij->h = ikkuna_h - h[i] - sij->y;
	    piirrä(0);
	    if(maxw < (tot=&tkstoliot[iii]->toteutuma)->w + xoff)
		maxw = tot->w + xoff;
	    tot_ = tot;
	}
	if(++ind>=pit) break;
	iii = oliojärjestys[ind];
	x += maxw + sivutilaa;
    }
    laitot = laitot0;
    return laitetut;
}

void tuhoa_asetelma() {
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
    TTF_CloseFont(tilastotol.font);
    TTF_CloseFont(sektusol.font);
    TTF_CloseFont(tkstalol.font);
    TTF_CloseFont(lisaol.font);
    TTF_CloseFont(valikkool.font);
}

SDL_Renderer* rend;
SDL_Window* ikkuna;
SDL_Texture* tausta;
