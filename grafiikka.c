#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <listat.h>
#include "rakenteet.h"
#include "grafiikka.h"

#define PYYHI(olio) SDL_RenderFillRect(k->rend, k->olio->toteutuma)
#define KELLO (k->kello_o->teksti)
#define LAITOT (*(k->laitot))

float skaala = 1.0;

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
  if(LAITOT.jarj) {
    PYYHI(jarj1_o);
    PYYHI(jarj2_o);
  }
  if(LAITOT.tiedot) {
    PYYHI(tiedot_o);
    PYYHI(tluvut_o);
  }
  if(LAITOT.lisatd)
    PYYHI(lisa_o);
  if(LAITOT.muut)
    PYYHI(muut_o);
  if(LAITOT.tkstal)
    PYYHI(tkstal_o);

  /*laitetaan uudet*/
  if(LAITOT.kello) {
    if(KELLO[0])
      laita_teksti_ttf(k->kello_o, k->rend);
    LAITOT.kello = 0;
  }
  if(LAITOT.valinta) {
    laita_valinta(k->vnta_o, k->rend);
    LAITOT.valinta = 0;
  }
  if(LAITOT.muut) {
    laita_vierekkain(k->muut_a, k->muut_b, 0, k->muut_o, k->rend);
    LAITOT.muut = 0;
  }
  if(LAITOT.sektus) {
    laita_tekstilista(_yalkuun(k->sekoitukset), 1, k->sektus_o, k->rend);
    LAITOT.sektus = 0;
  }
  if(LAITOT.tulos) {
    laita_oikealle(k->kello_o, 10, _yalkuun(k->tkset->strtulos), 1, k->tulos_o, k->rend);
    LAITOT.tulos = 0;
  }
  if(LAITOT.tkstal) {
    laita_teksti_ttf_vasemmalle(k->tulos_o, 10, k->tkstal_o, k->rend);
    LAITOT.tkstal = 0;
  }
  if(LAITOT.jarj) {
    strlista* a = _ynouda(_yalkuun(k->tkset->sijarj), 1);
    strlista* b = _ynouda(_yalkuun(k->tkset->strjarj), 1);
    int n = laita_pari_oikealle(k->tulos_o, 20,		\
				a, b, 0,		\
				k->jarj1_o, k->rend);
    a = _ynouda(a, n);
    b = _ynouda(b, n);
    laita_pari_oikealle(k->tulos_o, 20, a, b, 1, k->jarj2_o, k->rend);
    k->jarj2_o->alku += n; //listaa ei ollut annettu alusta asti
    if(k->jarj1_o->toteutuma->w < k->jarj2_o->toteutuma->w)
      k->jarj1_o->toteutuma->w = k->jarj2_o->toteutuma->w;
    LAITOT.jarj = 0;
  }
  if(LAITOT.tiedot) {
    /*tässä muuttujien nimet ovat aivan epäloogiset*/
    laita_oikealle(k->jarj1_o, 25, k->tietoalut, 1, k->tiedot_o, k->rend);
    laita_oikealle(k->tiedot_o, 0, _yalkuun(k->tiedot), 1, k->tluvut_o, k->rend);
    LAITOT.tiedot = 0;
  }
  if(LAITOT.lisatd) {
    laita_oikealle(k->jarj1_o, 20, _yalkuun(k->lisatd), 0, k->lisa_o, k->rend);
    LAITOT.lisatd = 0;
  }
  SDL_RenderPresent(k->rend);
}

void laita_teksti_ttf(tekstiolio_s *o, SDL_Renderer *rend) {
  SDL_Surface *pinta;
  switch(o->ttflaji) {
  case 0:
  OLETUSLAJI:
    pinta = TTF_RenderUTF8_Solid(o->font, o->teksti, o->vari);
    break;
  case 1:
    pinta = TTF_RenderUTF8_Shaded(o->font, o->teksti, o->vari, (SDL_Color){0,0,0,0});
    break;
  case 2:
    pinta = TTF_RenderUTF8_Blended(o->font, o->teksti, o->vari);
    break;
  default:
    printf("Varoitus: tekstin laittamisen laji on tuntematon, käytetään oletusta\n");
    goto OLETUSLAJI;
  }
  if(!pinta) {
    fprintf(stderr, "Virhe tekstin luomisessa: %s\n", TTF_GetError());
    return;
  }
  SDL_Texture *ttuuri = SDL_CreateTextureFromSurface(rend, pinta);
  if(!ttuuri)
    fprintf(stderr, "Virhe tekstuurin luomisessa: %s\n", SDL_GetError());

  /*kuvan koko on luodun pinnan koko, mutta enintään objektille määritelty koko
    tulostetaan vain se osa lopusta, joka mahtuu kuvaan*/

  *(o->toteutuma) = (SDL_Rect){o->sij->x*skaala,			\
			       o->sij->y*skaala,			\
			       (pinta->w < o->sij->w)? pinta->w : o->sij->w, \
			       (pinta->h < o->sij->h)? pinta->h : o->sij->h};
  o->toteutuma->w *= skaala;
  o->toteutuma->h *= skaala;

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

/*antamalla aluksi (alku) 0:n lista tulostetaan alkupäästä, muuten loppupäästä
  palauttaa, montako laitettiin*/
int laita_tekstilista(strlista* l, int alku, tekstiolio_s *o, SDL_Renderer *rend) {
  if(!l) {
    o->toteutuma->w = 0;
    o->toteutuma->h = 0;
    return 0;
  }
  int rvali = TTF_FontLineSkip(o->font);
  int mahtuu = o->sij->h / rvali;
  int yht = _ylaske(l) - o->rullaus;
  /*tässä toteutumaksi tulee maksimit*/
  int maksw = 0;
  int montako = 0;
  
  /*laitetaan niin monta jäsentä kuin mahtuu*/
  if(alku) //laitetaan lopusta
    alku = (mahtuu > yht)? 0 : (yht - mahtuu);
  else //laitetaan alusta
    alku = -o->rullaus;
  o->alku = alku;
  l = _ynouda(l, alku);
  int oy = o->sij->y;
  for(int i=0; i<mahtuu && l; i++) {
    if(o->numerointi) {
      o->teksti = malloc(strlen(l->str)+10);
      sprintf(o->teksti, "%i. %s", alku+1+i, l->str);
    } else {
      o->teksti = l->str;
    }
    laita_teksti_ttf(o, rend);
    montako++;
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
  return montako;
}

/*olion oikealle laitetaan kaksi listaa yhdessä (erikseen numerointi ja ajat)*/
int laita_pari_oikealle(tekstiolio_s* ov, int vali,		\
			   strlista* l1, strlista* l2, int alku,	\
			   tekstiolio_s* o, SDL_Renderer* rend) {
  SDL_Rect sij0 = *(o->sij);
  SDL_Rect tot1;
  int uusi_x = ov->toteutuma->x + ov->toteutuma->w + vali;
  if(o->sij->x < uusi_x)
    o->sij->x = uusi_x;
  
  int montako = laita_tekstilista(l1, alku, o, rend);
  tot1 = *(o->toteutuma);
  o->sij->x = o->toteutuma->x + o->toteutuma->w;
  o->sij->w -= o->toteutuma->w;
  laita_tekstilista(l2, alku, o, rend);
  *(o->sij) = sij0;
  o->toteutuma->x = tot1.x;
  o->toteutuma->w += tot1.w;
  return montako;
}

void laita_valinta(vnta_s* o, SDL_Renderer *rend) {
  if(o->valittu)
    SDL_RenderCopy(rend, o->kuvat->valittu, NULL, o->kuvat->sij);
  else
    SDL_RenderCopy(rend, o->kuvat->ei_valittu, NULL, o->kuvat->sij);
  laita_teksti_ttf(o->teksti, rend);
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

/*tämä palauttaa toteutumaksi näitten yhteisen alueen*/
void laita_vierekkain(strlista* a, strlista* b, int alku, tekstiolio_s* o, SDL_Renderer* r) {
  laita_tekstilista(a, alku, o, r);
  SDL_Rect sij0 = *(o->sij);
  SDL_Rect tot0 = *(o->toteutuma);
  
  o->sij->x = o->toteutuma->x + o->toteutuma->w;
  o->sij->y = o->toteutuma->y;
  o->sij->w = o->sij->w - o->toteutuma->w;
  o->sij->h = o->toteutuma->h;

  laita_tekstilista(b, alku, o, r);
  
  *(o->sij) = sij0;
  o->toteutuma->x = tot0.x;
  o->toteutuma->w += tot0.w;
  if(o->toteutuma->h < tot0.h)
    o->toteutuma->h = tot0.h;
}

void laita_oikealle(tekstiolio_s* ov, short vali, strlista* l, int alku, tekstiolio_s* o, SDL_Renderer* r) {
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

void laita_teksti_ttf_vasemmalle(tekstiolio_s* ov, short vali, tekstiolio_s* o, SDL_Renderer* r) {
  if(!strcmp(o->teksti, ""))
    return;
  if(!o)
    o = ov;
  SDL_Surface *pinta;
  switch(o->ttflaji) {
  case 0:
  OLETUSLAJI:
    pinta = TTF_RenderUTF8_Solid(o->font, o->teksti, o->vari);
    break;
  case 1:
    pinta = TTF_RenderUTF8_Shaded(o->font, o->teksti, o->vari, (SDL_Color){0,0,0,0});
    break;
  case 2:
    pinta = TTF_RenderUTF8_Blended(o->font, o->teksti, o->vari);
    break;
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

  /*kuvan koko on luodun pinnan koko, mutta enintään objektille määritelty koko
    tulostetaan vain se osa lopusta, joka mahtuu kuvaan*/

  /*kumpi tahansa, x tai w voi rajoittaa tätä*/
  int yrite = ov->sij->x - vali - pinta->w;
  if(pinta->w > o->sij->w)
    yrite = ov->sij->x - vali - o->sij->w;
  *(o->toteutuma) = (SDL_Rect){(yrite > o->sij->x)? yrite : o->sij->x,	\
			       o->sij->y,				\
			       (o->sij->w < pinta->w)? o->sij->w : pinta->w, \
			       (pinta->h < o->sij->h)? pinta->h : o->sij->h};

  yrite = pinta->w - o->toteutuma->w;
  SDL_Rect osa = (SDL_Rect){(yrite>0)? yrite : 0,			\
			    (pinta->h < o->sij->h)? 0 : pinta->h - o->toteutuma->h, \
			    pinta->w - ((yrite>0)? yrite : 0),		\
			    pinta->h};

  SDL_RenderFillRect(r, o->toteutuma);
  SDL_RenderCopy(r, ttuuri, &osa, o->toteutuma);
  SDL_FreeSurface(pinta);
  SDL_DestroyTexture(ttuuri);
  return;
}
