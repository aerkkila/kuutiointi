#ifndef __grafiikka__
#define __grafiikka__

#include <SDL2/SDL_ttf.h>
#include "listat.h"

typedef struct {
    char* teksti;
    int ttflaji; //mitä ttf-kirjaston funktiota käytetään (katso laita_teksti_ttf())
    TTF_Font* font;
    int fonttikoko;
    char* fonttied;
    SDL_Rect sij; //tälle varattu maksimitila
    SDL_Rect toteutuma; //mikä tila oikeasti käytetään
    int x, y; // jätetäänkö ylimääräistä tilaa vasemmalle tai ylös
    SDL_Color vari;
    short alku; //koskee vain listoja, ensimmäisen näytetyn indeksi, 'r'
    short rullaus; //koskee vain listoja, 'w'
    char numerointi; //koskee vain listoja;
} tekstiolio_s;

typedef struct {
    SDL_Rect kulmio;
    int paksuus;
    SDL_Color vari;
} korostustietue;

#endif

extern unsigned laitot;
extern const unsigned kellolai;
extern const unsigned sektuslai;
extern const unsigned tuloslai;
extern const unsigned jarj1lai;
extern const unsigned jarj2lai;
extern const unsigned jarjlai;
extern const unsigned tilastotlai;
extern const unsigned lisatdlai;
extern const unsigned valikkolai;
extern const unsigned tkstallai;
extern const unsigned korostuslai;
extern const unsigned kaikki_laitot;
extern const unsigned jäädytä;

int xsijainti(tekstiolio_s* ol, int p);
void saada_kohdistin();
void piirrä(int päivitä);
void laita_teksti_ttf(tekstiolio_s*, SDL_Renderer*);
int laita_tekstilista(slista*, int, tekstiolio_s*, SDL_Renderer*);
void laita_teksti_ttf_vasemmalle(tekstiolio_s* ov, short vali, tekstiolio_s* o, SDL_Renderer* r);
