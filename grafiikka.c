#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <listat.h>
#include "asetelma.h"
#include "grafiikka.h"
#include "tulokset.h"

#define PYYHI(olio) SDL_RenderFillRect(rend, &olio.toteutuma)
#define KELLO (kellool.teksti)

unsigned short laitot = 0x01ff;
const unsigned short kellolai  = 0x0001;
const unsigned short sektuslai = 0x0002;
const unsigned short tuloslai  = 0x0004;
const unsigned short jarjlai   = 0x0008;
const unsigned short tiedtlai  = 0x0010;
const unsigned short lisatdlai = 0x0020;
const unsigned short muutlai   = 0x0040;
const unsigned short tkstallai = 0x0080;
const unsigned short vntalai   = 0x0100;
const unsigned short muuta_tulos = 0x3e; //sektuslai | tuloslai | jarjlai | tiedtlai | lisatdlai;
const unsigned short kaikki_laitot = 0x01ff;

float skaala = 1.0;

void piirra() {
  /*pyyhitään vanhat*/
  if(laitot & kellolai)
    PYYHI(kellool);
  if(laitot & vntalai) {
    SDL_RenderFillRect(rend, &tarknap.kuvat.sij);
    PYYHI(tarknap.teksti);
  }
  if(laitot & sektuslai)
    PYYHI(sektusol);
  if(laitot & tuloslai)
    PYYHI(tulosol);
  if(laitot & jarjlai) {
    PYYHI(jarjol1);
    PYYHI(jarjol2);
  }
  if(laitot & tiedtlai) {
    PYYHI(tiedotol);
    PYYHI(tluvutol);
  }
  if(laitot & lisatdlai)
    PYYHI(lisaol);
  if(laitot & muutlai)
    PYYHI(muutol);
  if(laitot & tkstallai)
    PYYHI(tkstalol);

  /*laitetaan uudet*/
  if(laitot & kellolai) {
    if(KELLO[0])
      laita_teksti_ttf(&kellool, rend);
    laitot &= ~kellolai;
  }
  if(laitot & vntalai) {
    laita_valinta(&tarknap, rend);
    laitot &= ~vntalai;
  }
  if(laitot & muutlai) {
    laita_vierekkain(muut_a, muut_b, 0, &muutol, rend);
    laitot &= ~muutlai;
  }
  if(laitot & sektuslai) {
    laita_tekstilista(_yalkuun(sektus), 1, &sektusol, rend);
    laitot &= ~sektuslai;
  }
  if(laitot & tuloslai) {
    laita_oikealle(&kellool, 10, _yalkuun(strtulos), 1, &tulosol, rend);
    laitot &= ~tuloslai;
  }
  if(laitot & tkstallai) {
    laita_teksti_ttf_vasemmalle(&tulosol, 10, &tkstalol, rend);
    laitot &= ~tkstallai;
  }
  if(laitot & jarjlai) {
    strlista* a = _ynouda(_yalkuun(sijarj), 1);
    strlista* b = _ynouda(_yalkuun(strjarj), 1);
    int n = laita_pari_oikealle(&tulosol, 20, a, b, 0, &jarjol1, rend);
    a = _ynouda(a, n);
    b = _ynouda(b, n);
    laita_pari_oikealle(&tulosol, 20, a, b, 1, &jarjol2, rend);
    jarjol2.alku += n; //listaa ei ollut annettu alusta asti
    if(jarjol1.toteutuma.w < jarjol2.toteutuma.w)
      jarjol1.toteutuma.w = jarjol2.toteutuma.w;
    laitot &= ~jarjlai;
  }
  if(laitot & tiedtlai) {
    /*tässä muuttujien nimet ovat aivan epäloogiset*/
    laita_oikealle(&jarjol1, 25, tietoalut, 1, &tiedotol, rend);
    laita_oikealle(&tiedotol, 0, _yalkuun(tiedot), 1, &tluvutol, rend);
    laitot &= ~tiedtlai;
  }
  if(laitot & lisatdlai) {
    laita_oikealle(&jarjol1, 20, _yalkuun(lisatd), 0, &lisaol, rend);
    laitot &= ~lisatdlai;
  }
  SDL_RenderPresent(rend);
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

  o->toteutuma = (SDL_Rect){o->sij.x*skaala,				\
			    o->sij.y*skaala,				\
			    (pinta->w < o->sij.w)? pinta->w : o->sij.w,	\
			    (pinta->h < o->sij.h)? pinta->h : o->sij.h};
  o->toteutuma.w *= skaala;
  o->toteutuma.h *= skaala;

  SDL_Rect osa = {(pinta->w < o->sij.w)? 0 : pinta->w - o->toteutuma.w, \
		  (pinta->h < o->sij.h)? 0 : pinta->h - o->toteutuma.h, \
		  pinta->w,						\
		  pinta->h};

  SDL_RenderFillRect(rend, &o->toteutuma);
  SDL_RenderCopy(rend, ttuuri, &osa, &o->toteutuma);
  SDL_FreeSurface(pinta);
  SDL_DestroyTexture(ttuuri);
  return;
}

/*antamalla aluksi (alku) 0:n lista tulostetaan alkupäästä, muuten loppupäästä
  palauttaa, montako laitettiin*/
int laita_tekstilista(strlista* l, int alku, tekstiolio_s *o, SDL_Renderer *rend) {
  if(!l) {
    o->toteutuma.w = 0;
    o->toteutuma.h = 0;
    return 0;
  }
  int rvali = TTF_FontLineSkip(o->font);
  int mahtuu = o->sij.h / rvali;
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
  int oy = o->sij.y;
  for(int i=0; i<mahtuu && l; i++) {
    if(o->numerointi) {
      o->teksti = malloc(strlen(l->str)+10);
      sprintf(o->teksti, "%i. %s", alku+1+i, l->str);
    } else {
      o->teksti = l->str;
    }
    laita_teksti_ttf(o, rend);
    montako++;
    if(o->toteutuma.w > maksw)
      maksw = o->toteutuma.w;
    (o->sij.y) += rvali;
    l = l->seur;
    if(o->numerointi)
      free(o->teksti);
  }
  o->toteutuma.x = o->sij.x;
  o->toteutuma.y = oy;
  o->toteutuma.w = maksw;
  o->toteutuma.h = o->sij.y - oy;
  o->sij.y = oy;
  return montako;
}

/*olion oikealle laitetaan kaksi listaa yhdessä (erikseen numerointi ja ajat)*/
int laita_pari_oikealle(tekstiolio_s* ov, int vali,		\
			   strlista* l1, strlista* l2, int alku,	\
			   tekstiolio_s* o, SDL_Renderer* rend) {
  SDL_Rect sij0 = o->sij;
  SDL_Rect tot1;
  int uusi_x = ov->toteutuma.x + ov->toteutuma.w + vali;
  if(o->sij.x < uusi_x)
    o->sij.x = uusi_x;
  
  int montako = laita_tekstilista(l1, alku, o, rend);
  tot1 = o->toteutuma;
  o->sij.x = o->toteutuma.x + o->toteutuma.w;
  o->sij.w -= o->toteutuma.w;
  laita_tekstilista(l2, alku, o, rend);
  o->sij = sij0;
  o->toteutuma.x = tot1.x;
  o->toteutuma.w += tot1.w;
  return montako;
}

void laita_valinta(vnta_s* o, SDL_Renderer *rend) {
  if(o->valittu)
    SDL_RenderCopy(rend, o->kuvat.valittu, NULL, &o->kuvat.sij);
  else
    SDL_RenderCopy(rend, o->kuvat.ei_valittu, NULL, &o->kuvat.sij);
  laita_teksti_ttf(&(o->teksti), rend);
  return;
}

void laita_tiedot(strlista* a, tekstiolio_s* oa,			\
		  strlista* b, tekstiolio_s* ob, SDL_Renderer* r) {
  laita_tekstilista(a, 1, oa, r);
  ob->sij.x = oa->toteutuma.x + oa->toteutuma.w;
  ob->sij.y = oa->toteutuma.y;
  ob->sij.w = oa->sij.w - oa->toteutuma.w;
  ob->sij.h = oa->toteutuma.h;
  laita_tekstilista(_yalkuun(b), 1, ob, r);
  return;
}

/*tämä palauttaa toteutumaksi näitten yhteisen alueen*/
void laita_vierekkain(strlista* a, strlista* b, int alku, tekstiolio_s* o, SDL_Renderer* r) {
  laita_tekstilista(a, alku, o, r);
  SDL_Rect sij0 = o->sij;
  SDL_Rect tot0 = o->toteutuma;
  
  o->sij.x = o->toteutuma.x + o->toteutuma.w;
  o->sij.y = o->toteutuma.y;
  o->sij.w = o->sij.w - o->toteutuma.w;
  o->sij.h = o->toteutuma.h;

  laita_tekstilista(b, alku, o, r);
  
  o->sij = sij0;
  o->toteutuma.x = tot0.x;
  o->toteutuma.w += tot0.w;
  if(o->toteutuma.h < tot0.h)
    o->toteutuma.h = tot0.h;
}

void laita_oikealle(tekstiolio_s* ov, short vali, strlista* l, int alku, tekstiolio_s* o, SDL_Renderer* r) {
  if(!o)
    o = ov;
  int vanha_x = o->sij.x;
  int uusi_x = ov->toteutuma.x + ov->toteutuma.w + vali;
  if(o->sij.x < uusi_x)
    o->sij.x = uusi_x;
  laita_tekstilista(l, alku, o, r);
  o->sij.x = vanha_x;
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
  int yrite = ov->sij.x - vali - pinta->w;
  if(pinta->w > o->sij.w)
    yrite = ov->sij.x - vali - o->sij.w;
  o->toteutuma = (SDL_Rect) {(yrite > o->sij.x)? yrite : o->sij.x,	\
			     o->sij.y,					\
			     (o->sij.w < pinta->w)? o->sij.w : pinta->w, \
			     (pinta->h < o->sij.h)? pinta->h : o->sij.h};
  
  yrite = pinta->w - o->toteutuma.w;
  SDL_Rect osa = {(yrite>0)? yrite : 0,					\
		  (pinta->h < o->sij.h)? 0 : pinta->h - o->toteutuma.h, \
		  pinta->w - ((yrite>0)? yrite : 0),			\
		  pinta->h};
  
  SDL_RenderFillRect(r, &o->toteutuma);
  SDL_RenderCopy(r, ttuuri, &osa, &o->toteutuma);
  SDL_FreeSurface(pinta);
  SDL_DestroyTexture(ttuuri);
  return;
}
