#ifndef __ASETELMA__
#define __ASETELMA__

#include <SDL2/SDL.h>
#include "listat.h"
#include "grafiikka.h"

#define TULOSKANSIO "skello"
#define KOTIKANSIO "/usr/share/skello"

enum oliot {
    kello_oe, sektus_oe, tulos_oe, jarj_oe,
    tilastot_oe, lisä_oe, valikko_oe,
    tkstal_oe, Viimeinen_oe, Null_oe,
};

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
enum {tarkasteluaika_e, ulosnimi_e, eri_sekunnit_e, kuvaaja_e, kuutio_e, aani_e, autokuutio_e}; // valikon järjestys
extern const char* tarkastelu_str[];
extern int tarkasteluaikatila;
extern const char* aanivaihtoehdot[];
enum aanivaihtoehto {aani_pois_e, aani_kuuntelu_e, ääni_pysäytys_e, ääni_vaihtoehtoja};
extern enum aanivaihtoehto äänitila;
extern char* äänitila_str;
extern const char* tekstialue[];
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
extern tekstiolio_s tilastotol;
extern tekstiolio_s tilastoluvutol;
extern tekstiolio_s lisaol;
extern tekstiolio_s sektusol;
extern tekstiolio_s valikkool;
extern tekstiolio_s tkstalol;
extern korostustietue korostusol;

extern SDL_Color kellovärit[];

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
unsigned vakiosijainnit();
void tuhoa_asetelma();

#endif
