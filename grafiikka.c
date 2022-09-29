#include <stdio.h>
#include <SDL2/SDL.h>
#include "asetelma.h"
#include "grafiikka.h"
#include "tulokset.h"

#define PYYHI(olio) SDL_RenderFillRect(rend, &olio.toteutuma)
#define KELLO (kellool.teksti)
#define lista_alusta 0
#define lista_lopusta 1

/**
   Muuttuja #laitot määrittää, mitkä osat piirretään uudestaan.
   Jos #laitot == #jäädytä, koko kuva piirretään #tausta-tekstuuriin
   Jos #laitot == #0, tausta vain kopioidaan näytölle.
   Jos #laitot == #kaikki_laitot, koko kuva paitsi kello piirretäan #tausta-tekstuuriin
   Jos #laitot == #kellolai, tausta kopioidaan näytölle ja laitetaan kello päälle.
   Jos #laitot & #JOTAINlai, kyseinen JOTAIN pyyhitään taustasta ja piirretään uudestaan
   Käyttöliittymässä asetettakoon tämän jälkeen aina #laitot = #kellolai * (#tila != #seis)
*/

void laita_järjestysolio();
void laita_ehkä_korostus();
unsigned laitot;
const unsigned kellolai  = 1<<kello_oe;
const unsigned sektuslai = 1<<sektus_oe;
const unsigned tuloslai  = 1<<tulos_oe;
const unsigned jarjlai   = 1<<jarj_oe;
const unsigned tilastotlai = 1<<tilastot_oe;
const unsigned lisatdlai = 1<<lisä_oe;
const unsigned valikkolai   = 1<<valikko_oe;
const unsigned tkstallai = 1<<tkstal_oe;
const unsigned kaikki_laitot = (1<<Viimeinen_oe) - 1;
const unsigned jäädytä   = (1<<(Viimeinen_oe+1)) - 1; // myös kello laitetaan taustaan

float skaala = 1.0;
SDL_Rect kohdistinsij;
extern int kohdistin;

int xsijainti(tekstiolio_s* o, int p) {
    int lev;
    char c0 = o->teksti[p];
    o->teksti[p] = '\0';
    TTF_SizeUTF8(o->font, o->teksti, &lev, NULL);
    o->teksti[p] = c0;
    return lev + o->toteutuma.x;
}

void säädä_kohdistin() {
    kohdistinsij.y = kellool.sij.y;
    kohdistinsij.h = TTF_FontLineSkip(kellool.font);
    TTF_GlyphMetrics(kellool.font, ' ', NULL, NULL, NULL, NULL, &kohdistinsij.w);
    kohdistinsij.w /= 10;
    if(!kohdistinsij.w)
	kohdistinsij.w = 1;
}

void aseta_väri(SDL_Renderer* rend, SDL_Color* vari) {
    SDL_SetRenderDrawColor(rend, vari->g, vari->b, vari->b, vari->a);
}

void laita_ehkä_kohdistin() {
    if(kohdistin < 0) return;
    säädä_kohdistin();
    aseta_väri(rend, &kohdistinvari);
    kohdistinsij.x = xsijainti(&kellool, strlen(kellool.teksti)-kohdistin);
    SDL_RenderFillRect(rend, &kohdistinsij);
}

/* Argumentti huomioidaan päivityksenä vain, jos laitot==kellolai.
   Muuten se käsitellään pyyhintäkieltona */
void piirrä(int päivitä) {
    if(laitot == kellolai) {
	if(!päivitä) {
	    if(KELLO[0]) laita_teksti_ttf(&kellool, rend);
	    return;
	}
	PYYHI(kellool);
	SDL_RenderCopy(rend, tausta, NULL, NULL);
	if(KELLO[0]) laita_teksti_ttf(&kellool, rend);
	laita_ehkä_kohdistin();
	laita_ehkä_korostus();
	SDL_RenderPresent(rend);
	return;
    }
    if(!laitot) {
	SDL_RenderCopy(rend, tausta, NULL, NULL);
	laita_ehkä_korostus();
	SDL_RenderPresent(rend);
	return;
    }
    SDL_SetRenderTarget(rend, tausta);
    aseta_väri(rend, &taustavari);
    if((laitot & kaikki_laitot) == kaikki_laitot) {
	vakiosijainnit();
	SDL_SetRenderTarget(rend, tausta);
	SDL_RenderClear(rend);
    }

    if((laitot & kaikki_laitot) == kaikki_laitot || !päivitä) goto laittaos;
    if(laitot & kellolai)    PYYHI(kellool);
    if(laitot & sektuslai)   PYYHI(sektusol);
    if(laitot & tuloslai)    PYYHI(tulosol);
    if(laitot & jarjlai)     { PYYHI(jarjol1); PYYHI(jarjol2); }
    if(laitot & tilastotlai) { PYYHI(tilastotol); PYYHI(tilastoluvutol); }
    if(laitot & valikkolai)  PYYHI(valikkool);
    if(laitot & lisatdlai)   PYYHI(lisaol);
    if(laitot & tkstallai)   PYYHI(tkstalol);

laittaos:
    if(laitot & valikkolai)
	laita_tekstilista(muut_a, 0, &valikkool, rend);
    if(laitot & sektuslai)
	laita_tekstilista(sektus, 1, &sektusol, rend);
    if(laitot & tuloslai)
	laita_tekstilista(stulos, 1, &tulosol, rend);
    if(laitot & jarjlai)
	laita_järjestysolio();
    if(laitot & tilastotlai) {
	laita_tekstilista(tietoalut, 1, &tilastotol, rend);
	tilastoluvutol.sij.x = tilastotol.toteutuma.x + tilastotol.toteutuma.w;
	tilastoluvutol.sij.y = tilastotol.toteutuma.y;
	laita_tekstilista(tietoloput, 1, &tilastoluvutol, rend);
    }
    if(laitot & lisatdlai)
	laita_tekstilista(lisatd, 0, &lisaol, rend);
    if(laitot & tkstallai)
	laita_teksti_ttf_vasemmalle(&tulosol, 10, &tkstalol, rend); 

    if(!päivitä) return;

    /* Kelloa ei laiteta taustaan ellei jäädytetä. Tässä se laitetaan suoraan näytölle. */
    if(laitot != jäädytä) {
	if(laitot != kaikki_laitot) goto valmis;
	SDL_SetRenderTarget(rend, NULL);
	SDL_RenderCopy(rend, tausta, NULL, NULL);
	if(KELLO[0])
	    laita_teksti_ttf(&kellool, rend);
	laita_ehkä_kohdistin();
	laita_ehkä_korostus();
	SDL_RenderPresent(rend);
	return;
    }
    if(KELLO[0]) // jäädytä
	laita_teksti_ttf(&kellool, rend);
    laita_ehkä_kohdistin(); // tuskin laitetaan
valmis:
    SDL_SetRenderTarget(rend, NULL);
    SDL_RenderCopy(rend, tausta, NULL, NULL);
    laita_ehkä_korostus();
    SDL_RenderPresent(rend);
}

SDL_Surface* luo_tekstipinta0(tekstiolio_s *o) {
    return TTF_RenderUTF8_Solid(o->font, o->teksti, o->vari); }
SDL_Surface* luo_tekstipinta1(tekstiolio_s *o) {
    return TTF_RenderUTF8_Shaded(o->font, o->teksti, o->vari, (SDL_Color){0,0,0,0}); }
SDL_Surface* luo_tekstipinta2(tekstiolio_s *o) {
    return TTF_RenderUTF8_Blended(o->font, o->teksti, o->vari); }

SDL_Surface* (*luo_tekstipinta[])(tekstiolio_s*) = {
    luo_tekstipinta0, luo_tekstipinta1, luo_tekstipinta2
};

void laita_teksti_ttf(tekstiolio_s *o, SDL_Renderer *rend) {
    SDL_Texture *ttuuri;
    SDL_Surface *pinta;
    int rajaus = -1;
    char kirjain = '\0';
    do {
	pinta = luo_tekstipinta[o->ttflaji](o);
	if(!pinta) {
	    fprintf(stderr, "Virhe tekstin luomisessa: %s\n", TTF_GetError());
	    return;
	}
	if((ttuuri = SDL_CreateTextureFromSurface(rend, pinta)))
	    goto tekstuuri_luotu;

	/* Ellei tekstuurin luominen onnistunut, teksti on luultavasti liian pitkä.
	   Puolitettakoon tekstin pituutta kunnes onnistuu tai teksti loppuu. */
	SDL_FreeSurface(pinta);
	pinta = NULL;
	int seuraava_rajaus = strlen(o->teksti) / 2;
	if(rajaus >= 0)
	    o->teksti[rajaus] = kirjain;
	if(!seuraava_rajaus) break;
	rajaus = seuraava_rajaus;
	kirjain = o->teksti[rajaus];
	o->teksti[rajaus] = '\0';
    } while(1);
    fprintf(stderr, "Virhe tekstuurin luomisessa: %s\n", SDL_GetError());
    return;

tekstuuri_luotu:
    if(rajaus >= 0)
	o->teksti[rajaus] = kirjain;

    /* Jos o->sij.[wh] on 0, alueen koko saa olla vapaasti koko ikkunan suuruinen.
       Asetettakoon niihin siksi ikkunan koko, mutta tallennetaan mitä muutettiin muuttujaan hw0. */
    unsigned hw0 = 0;
    if(!o->sij.w) {
	o->sij.w = ikkuna_w - o->sij.x;
	hw0 |= 1<<0;
    }
    if(!o->sij.h) {
	o->sij.h = ikkuna_h - o->sij.y;
	hw0 |= 1<<1;
    }

    if(o->monirivinen) goto monirivinen;

    /* Tulostetaan vain se osa lopusta, joka mahtuu kuvaan. */
    o->toteutuma = (SDL_Rect){
	o->sij.x*skaala,
	o->sij.y*skaala,
	(pinta->w < o->sij.w)? pinta->w : o->sij.w,
	(pinta->h < o->sij.h)? pinta->h : o->sij.h
    };
    o->toteutuma.w *= skaala;
    o->toteutuma.h *= skaala;
    SDL_Rect osa = {
	(pinta->w < o->sij.w)? 0 : pinta->w - o->toteutuma.w,
	(pinta->h < o->sij.h)? 0 : pinta->h - o->toteutuma.h,
	pinta->w,
	pinta->h
    };

    /* Jos o->sij.[wh] oli nolla, palautetaan siksi, jotta myöhemmilläkään piirtokerroilla kokoa ei rajoiteta. */
    o->sij.w *= !(hw0 & 1<<0);
    o->sij.h *= !(hw0 & 1<<1);

    SDL_RenderCopy(rend, ttuuri, &osa, &o->toteutuma);
    SDL_FreeSurface(pinta);
    SDL_DestroyTexture(ttuuri);
    return;

monirivinen:
    /* Tulostetaan alusta niin pitkästi kuin mahtuu kuvaan,
       minkä jälkeen jatketaan seuraavalle riville. */
    int x = 0, jatka = 0;
    o->toteutuma = (SDL_Rect){
	.x = o->sij.x*skaala,
	.y = o->sij.y*skaala,
	.h = (pinta->h < o->sij.h)? pinta->h : o->sij.h
    };
    do {
	SDL_Rect osa = {
	    .x = x,
	    .y = pinta->h < o->sij.h ? 0 : pinta->h - o->toteutuma.h,
	    .h = pinta->h,
	};
	if(pinta->w-x > o->sij.w) {
	    osa.w = o->toteutuma.w = o->sij.w;
	    jatka = 1;
	}
	else {
	    osa.w = o->toteutuma.w = pinta->w - x;
	    jatka = 0;
	}
	SDL_RenderCopy(rend, ttuuri, &osa, &o->toteutuma);
	o->toteutuma.y += o->toteutuma.h;
	x += o->sij.w;
    } while(jatka);
    o->toteutuma.h = o->toteutuma.y - o->sij.y;
    o->toteutuma.y = o->toteutuma.y;
    o->toteutuma.w = pinta->w > o->sij.w ? o->sij.w : pinta->w;
    o->sij.w *= !(hw0 & 1<<0);
    o->sij.h *= !(hw0 & 1<<1);
    SDL_FreeSurface(pinta);
    SDL_DestroyTexture(ttuuri);
}

/*antamalla aluksi (alku) 0:n lista tulostetaan alkupäästä, muuten loppupäästä
  palauttaa, montako laitettiin*/
int laita_tekstilista(slista* sl, int alku, tekstiolio_s *o, SDL_Renderer *rend) {
    if(!sl) {
	o->toteutuma.w = 0;
	o->toteutuma.h = 0;
	return 0;
    }
    unsigned hw0 = 0; // Sijainniksi tarvittaessa ikkunan koko ja muistetaan, muutettiinko näitä.
    if(!o->sij.w) {
	o->sij.w = ikkuna_w - o->sij.x;
	hw0 |= 1<<0;
    }
    if(!o->sij.h) {
	o->sij.h = ikkuna_h - o->sij.y;
	hw0 |= 1<<1;
    }

    int rvali = TTF_FontLineSkip(o->font);
    int mahtuu = o->sij.h / rvali;
    int yht = sl->pit;
    int maksw = 0;

    /* laitetaan niin monta jäsentä kuin mahtuu */
    if(alku) // laitetaan lopusta
	o->alku = (mahtuu < yht)*(yht - mahtuu - o->rullaus); // 0, jos mahtuu >= yht
    else // laitetaan alusta
	o->alku = -o->rullaus;
    int o_sij_y0 = o->sij.y;
    int raja = (mahtuu < yht-o->alku)? mahtuu : yht-o->alku;
    int yraja = o->sij.y + o->sij.h;
    for(int i=0; i<raja && o->sij.y < yraja; i++) {
	if(o->numerointi) {
	    o->teksti = malloc(strlen( sl->taul[o->alku+i] )+10);
	    sprintf(o->teksti, "%i. %s", o->alku+1+i, sl->taul[o->alku+i]);
	}
	else
	    o->teksti = sl->taul[o->alku+i];
	laita_teksti_ttf(o, rend);
	if(o->toteutuma.w > maksw)
	    maksw = o->toteutuma.w;
	o->sij.y += o->toteutuma.h > rvali? o->toteutuma.h: rvali;
	if(o->numerointi)
	    free(o->teksti);
    }
    o->toteutuma.x = o->sij.x;
    o->toteutuma.y = o_sij_y0;
    o->toteutuma.w = maksw;
    o->toteutuma.h = o->sij.y - o_sij_y0;
    o->sij.y = o_sij_y0;
    o->sij.w *= !(hw0 & 1<<0);
    o->sij.h *= !(hw0 & 1<<1);
    return raja;
}

void laita_järjestysolio() {
    int rvali = TTF_FontLineSkip(jarjol1.font);
    int h_apu = jarjol1.sij.h * jarjsuhde;
    int maksw = 0;
    int i = -jarjol1.rullaus;
    jarjol1.alku = i;
    int o_sij_y0 = jarjol1.sij.y;
    for(; i<ftulos->pit && h_apu >= rvali; i++) {
	sprintf(jarjol1.teksti, "%i. %s", jarjes[i]+1, stulos->taul[jarjes[i]]);
	laita_teksti_ttf(&jarjol1, rend);
	jarjol1.sij.y += rvali;
	h_apu -= rvali;
	if(jarjol1.toteutuma.w > maksw)
	    maksw = jarjol1.toteutuma.w;
    }
    jarjol1.toteutuma.h = jarjol1.sij.y - o_sij_y0;
    jarjol1.toteutuma.y = o_sij_y0;
    jarjol1.toteutuma.w = maksw;
    jarjol1.toteutuma.x = jarjol1.sij.x;
    jarjol1.sij.y = o_sij_y0;

    /* jarjol2 */
    jarjol2.sij = jarjol1.sij;
    jarjol2.sij.y += jarjol1.toteutuma.h;
    jarjol2.sij.h -= jarjol1.toteutuma.h;
    int mahtuu = jarjol2.sij.h / rvali;
    int raja = (mahtuu < ftulos->pit-i)? mahtuu : ftulos->pit-i;
    i = ftulos->pit-raja;
    i -= jarjol2.rullaus;
    jarjol2.alku = i;
    for(int j=0; j<raja; j++, i++) {
	sprintf(jarjol2.teksti, "%i. %s", jarjes[i]+1, stulos->taul[jarjes[i]]);
	laita_teksti_ttf(&jarjol2, rend);
	jarjol2.sij.y += rvali;
	if(jarjol2.toteutuma.w > maksw)
	    maksw = jarjol2.toteutuma.w;
    }
    jarjol2.toteutuma.y = jarjol1.toteutuma.y + jarjol1.toteutuma.h;
    jarjol2.toteutuma.h = rvali*raja;
    jarjol2.toteutuma.w = maksw;
    jarjol1.toteutuma.w = maksw;
}

void laita_ehkä_korostus() {
    if(korostusol.paksuus <= 0) return;
    aseta_väri(rend, &korostusol.vari);
    SDL_Rect apu = korostusol.kulmio;
    for(int i=0; i<korostusol.paksuus; i++) {
	SDL_RenderDrawRect(rend, &apu);
	apu.x--;
	apu.y--;
	apu.w+=2;
	apu.h+=2;
    }
}

void laita_teksti_ttf_vasemmalle(tekstiolio_s* ov, short vali, tekstiolio_s* o, SDL_Renderer* r) {
    if(!o || !o->teksti || !strcmp(o->teksti, ""))
	return;
    SDL_Surface *pinta;
    switch(o->ttflaji) {
    case 0:
    OLETUSLAJI:
	pinta = TTF_RenderUTF8_Solid(o->font, o->teksti, o->vari); break;
    case 1:
	pinta = TTF_RenderUTF8_Shaded(o->font, o->teksti, o->vari, (SDL_Color){0,0,0,0}); break;
    case 2:
	pinta = TTF_RenderUTF8_Blended(o->font, o->teksti, o->vari); break;
    default:
	printf("Varoitus: tekstin laittamisen laji on tuntematon, käytetään oletusta\n");
	goto OLETUSLAJI;
    }
    if(!pinta) {
	fprintf(stderr, "Virhe tekstin luomisessa: %s\n", TTF_GetError());
	return;
    }
    SDL_Texture *ttuuri = SDL_CreateTextureFromSurface(r, pinta);
    if(!ttuuri)
	fprintf(stderr, "Virhe tekstuurin luomisessa: %s\n", SDL_GetError());

    int yrite = ov->sij.x - vali - pinta->w;
    int x = yrite>=0? yrite: 0;
    o->toteutuma = (SDL_Rect) {
	x,
	ov->sij.y,
	ov->sij.x - vali - x,
	pinta->h,
    };
    
    yrite = pinta->w - o->toteutuma.w;
    x = yrite>0? yrite : 0;
    SDL_Rect osa = {
	x,
	0,
	pinta->w - x,
	pinta->h
    };

    SDL_RenderFillRect(r, &o->toteutuma);
    SDL_RenderCopy(r, ttuuri, &osa, &o->toteutuma);
    SDL_FreeSurface(pinta);
    SDL_DestroyTexture(ttuuri);
    return;
}
