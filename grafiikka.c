#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <listat.h>
#include "rakenteet.h"
#include "grafiikka.h"

#define PYYHI(olio) SDL_RenderFillRect(k->rend, k->olio->toteutuma)
#define KELLO (k->kello_o->teksti)
#define LAITOT (*(k->laitot))

void piirra(kaikki_s* k) {
  /*pyyhitään vanhat*/
  if(LAITOT.kello)
    PYYHI(kello_o);
  if(LAITOT.valinta) {
    SDL_RenderFillRect(k->rend, k->vnta_o->kuvat->sij);
    PYYHI(vnta_o->teksti);
  }
  if(LAITOT.sektus)
    PYYHI(sektus_o);
  if(LAITOT.tulos)
    PYYHI(tulos_o);
  if(LAITOT.jarj)
    PYYHI(jarj_o);
  if(LAITOT.tiedot) {
    PYYHI(tiedot_o);
    PYYHI(tluvut_o);
  }
  if(LAITOT.lisatd)
    PYYHI(lisa_o);

  /*laitetaan uudet*/
  if(LAITOT.kello) {
    if(KELLO[0])
      laita_teksti_solid(k->kello_o, k->rend);
    LAITOT.kello = 0;
  }
  if(LAITOT.valinta) {
    laita_valinta(k->vnta_o, k->rend);
    LAITOT.valinta = 0;
  }
  if(LAITOT.sektus) {
    laita_tekstilista(_yalkuun(k->sekoitukset), 1, k->sektus_o, k->rend);
    LAITOT.sektus = 0;
  }
  if(LAITOT.tulos) {
    laita_vasemmalle(k->kello_o, 10, _yalkuun(k->tkset->strtulos), 1, k->tulos_o, k->rend);
    LAITOT.tulos = 0;
  }
  if(LAITOT.jarj) {
    laita_aaret(k->tulos_o, 20, _ynouda(_yalkuun(k->tkset->sijarj), 1),	\
		_ynouda(_yalkuun(k->tkset->strjarj), 1),			\
		k->jarj_o, k->jarjsuhde, k->rend);
    LAITOT.jarj = 0;
  }
  if(LAITOT.tiedot) {
    /*tässä muuttujien nimet ovat aivan epäloogiset*/
    laita_vasemmalle(k->jarj_o, 25, k->tietoalut, 1, k->tiedot_o, k->rend);
    laita_vasemmalle(k->tiedot_o, 0, _yalkuun(k->tiedot), 1, k->tluvut_o, k->rend);
    LAITOT.tiedot = 0;
  }
  if(LAITOT.lisatd) {
    laita_vasemmalle(k->jarj_o, 20, _yalkuun(k->lisatd), 1, k->lisa_o, k->rend);
    LAITOT.lisatd = 0;
  }
  SDL_RenderPresent(k->rend);
}

void laita_teksti_solid(tekstiolio_s *o, SDL_Renderer *rend) {
  SDL_Surface *pinta = TTF_RenderUTF8_Blended(o->font, o->teksti, o->vari);
  if(!pinta) {
    fprintf(stderr, "Virhe tekstin luomisessa: %s\n", TTF_GetError());
    return;
  }
  SDL_Texture *ttuuri = SDL_CreateTextureFromSurface(rend, pinta);
  if(!pinta)
    fprintf(stderr, "Virhe tekstuurin luomisessa: %s\n", SDL_GetError());

  /*kuvan koko on luodun pinnan koko, mutta enintään objektille määritelty koko
    tulostetaan vain se osa lopusta, joka mahtuu kuvaan*/

  *(o->toteutuma) = (SDL_Rect){o->sij->x,				\
			       o->sij->y,				\
			       (pinta->w < o->sij->w)? pinta->w : o->sij->w, \
			       (pinta->h < o->sij->h)? pinta->h : o->sij->h};

  SDL_Rect osa = (SDL_Rect){(pinta->w < o->sij->w)? 0 : pinta->w - o->toteutuma->w, \
			    (pinta->h < o->sij->h)? 0 : pinta->h - o->toteutuma->h, \
			    pinta->w,					\
			    pinta->h};

  SDL_RenderFillRect(rend, o->toteutuma);
  SDL_RenderCopy(rend, ttuuri, &osa, o->toteutuma);
  SDL_FreeSurface(pinta);
  SDL_DestroyTexture(ttuuri);
  return;
}

/*antamalla aluksi 0:n lista tulostetaan alkupäästä, muuten loppupäästä*/
void laita_tekstilista(strlista* l, int alku, tekstiolio_s *o, SDL_Renderer *rend) {
  if(!l) {
    o->toteutuma->w = 0;
    o->toteutuma->h = 0;
    return;
  }
  int rvali = TTF_FontLineSkip(o->font);
  int mahtuu = o->sij->h / rvali;
  int yht = _ylaske(l) - o->rullaus;
  /*tässä toteutumaksi tulee maksimit*/
  int maksw = 0;
  
  /*laitetaan niin monta jäsentä kuin mahtuu*/
  if(alku) //laitetaan lopusta
    alku = (mahtuu > yht)? 0 : (yht - mahtuu);
  o->alku = alku;
  l = _ynouda(l, alku);
  int oy = o->sij->y;
  for(int i=0; i<mahtuu && l; i++) {
    if(o->numerointi) {
      o->teksti = malloc(strlen(l->str)+5);
      sprintf(o->teksti, "%i. %s", alku+1+i, l->str);
    } else {
      o->teksti = l->str;
    }
    laita_teksti_solid(o, rend);
    if(o->toteutuma->w > maksw)
      maksw = o->toteutuma->w;
    (o->sij->y) += rvali;
    l = l->seur;
    if(o->numerointi)
      free(o->teksti);
  }
  o->toteutuma->x = o->sij->x;
  o->toteutuma->y = oy;
  o->toteutuma->w = maksw;
  o->toteutuma->h = o->sij->y - oy;
  o->sij->y = oy;
  return;
}

void laita_tekstilistan_paat(strlista* l, tekstiolio_s *o, float suhde, SDL_Renderer *rend) {
  if(!l) {
    o->toteutuma->w = 0;
    o->toteutuma->h = 0;
    return;
  }
  SDL_Rect sij0 = *(o->sij);
  SDL_Rect tot1;
  /*alkupää*/
  o->sij->h *= suhde;
  laita_tekstilista(l, 0, o, rend);
  tot1 = *(o->toteutuma);
  /*loppupää*/
  o->sij->h = sij0.h * (1 - suhde) - TTF_FontLineSkip(o->font);
  o->sij->y = o->toteutuma->h + o->toteutuma->y + TTF_FontLineSkip(o->font);
  laita_tekstilista(l, 1, o, rend);

  if(o->toteutuma->h == 0) o->toteutuma->y -= TTF_FontLineSkip(o->font);

  if(o->toteutuma->w < tot1.w) o->toteutuma->w = tot1.w;
  o->toteutuma->h = o->toteutuma->y + o->toteutuma->h - tot1.y;
  o->toteutuma->y = tot1.y;
  *(o->sij) = sij0;
  return;
}

void laita_aaret(tekstiolio_s* ov, short vali,			\
		 strlista* luvut, strlista* l, tekstiolio_s* o,	\
		 float suhde, SDL_Renderer* rend) {
  SDL_Rect sij0 = *(o->sij);
  SDL_Rect tot1;
  int uusi_x = ov->toteutuma->x + ov->toteutuma->w + vali;
  if(o->sij->x < uusi_x)
    o->sij->x = uusi_x;
  
  int rvali = TTF_FontLineSkip(o->font);
  int mahtuu = o->sij->h / rvali;
  laita_tekstilistan_paat(luvut, o, (mahtuu < _ylaske(l))? suhde : 1, rend);
  tot1 = *(o->toteutuma);
  o->sij->x = o->toteutuma->x + o->toteutuma->w;
  o->sij->w -= o->toteutuma->w;
  laita_tekstilistan_paat(l, o, (mahtuu < _ylaske(l))? suhde : 1, rend);
  *(o->sij) = sij0;
  o->toteutuma->x = tot1.x;
  o->toteutuma->w += tot1.w;
  return;
}

void laita_valinta(vnta_s* o, SDL_Renderer *rend) {
  if(o->valittu)
    SDL_RenderCopy(rend, o->kuvat->valittu, NULL, o->kuvat->sij);
  else
    SDL_RenderCopy(rend, o->kuvat->ei_valittu, NULL, o->kuvat->sij);
  laita_teksti_solid(o->teksti, rend);
  return;
}

void laita_tiedot(strlista* a, tekstiolio_s* oa,			\
		  strlista* b, tekstiolio_s* ob, SDL_Renderer* r) {
  laita_tekstilista(a, 1, oa, r);
  ob->sij->x = oa->toteutuma->x + oa->toteutuma->w;
  ob->sij->y = oa->toteutuma->y;
  ob->sij->w = oa->sij->w - oa->toteutuma->w;
  ob->sij->h = oa->toteutuma->h;
  laita_tekstilista(_yalkuun(b), 1, ob, r);
  return;
}

void laita_vierekkain(strlista* a, int ai, strlista* b, int bi,
		      tekstiolio_s* oa, tekstiolio_s* ob, SDL_Renderer* r) {
  laita_tekstilista(a, ai, oa, r);
  SDL_Rect sij = *(oa->sij);
  if(!ob)
    ob = oa;
  
  ob->sij->x = oa->toteutuma->x + oa->toteutuma->w;
  ob->sij->y = oa->toteutuma->y;
  ob->sij->w = oa->sij->w - oa->toteutuma->w;
  ob->sij->h = oa->toteutuma->h;

  laita_tekstilista(b, ai, ob, r);
  *(oa->sij) = sij;
}

void laita_vasemmalle(tekstiolio_s* ov, short vali, strlista* l, int alku, tekstiolio_s* o, SDL_Renderer* r) {
  if(!o)
    o = ov;
  int vanha_x = o->sij->x;
  int uusi_x = ov->toteutuma->x + ov->toteutuma->w + vali;
  if(o->sij->x < uusi_x)
    o->sij->x = uusi_x;
  laita_tekstilista(l, alku, o, r);
  o->sij->x = vanha_x;
  return;
}
