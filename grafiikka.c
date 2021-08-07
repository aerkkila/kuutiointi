#include <stdio.h>
#include <SDL2/SDL.h>
#include "asetelma.h"
#include "grafiikka.h"
#include "tulokset.h"

#define PYYHI(olio) SDL_RenderFillRect(rend, &olio.toteutuma)
#define KELLO (kellool.teksti)

void laita_jarjestus();
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
    laita_tekstilista(sektus, 1, &sektusol, rend);
    laitot &= ~sektuslai;
  }
  if(laitot & tuloslai) {
    laita_oikealle(&kellool, 10, stulos, 1, &tulosol, rend);
    laitot &= ~tuloslai;
  }
  if(laitot & tkstallai) {
    laita_teksti_ttf_vasemmalle(&tulosol, 10, &tkstalol, rend);
    laitot &= ~tkstallai;
  }
  if(laitot & jarjlai) {
    laita_jarjestus();
    laitot &= ~jarjlai;
  }
  if(laitot & tiedtlai) {
    /*tässä muuttujien nimet ovat aivan epäloogiset*/
    laita_oikealle(&jarjol1, 25, tietoalut, 1, &tiedotol, rend);
    laita_oikealle(&tiedotol, 0, tietoloput, 1, &tluvutol, rend);
    laitot &= ~tiedtlai;
  }
  if(laitot & lisatdlai) {
    laita_oikealle(&jarjol1, 20, lisatd, 0, &lisaol, rend);
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
int laita_tekstilista(slista* sl, int alku, tekstiolio_s *o, SDL_Renderer *rend) {
  if(!sl) {
    o->toteutuma.w = 0;
    o->toteutuma.h = 0;
    return 0;
  }
  int rvali = TTF_FontLineSkip(o->font);
  int mahtuu = o->sij.h / rvali;
  int yht = sl->pit - o->rullaus;
  /*tässä toteutumaksi tulee maksimit*/
  int maksw = 0;
  
  /*laitetaan niin monta jäsentä kuin mahtuu*/
  if(alku) //laitetaan lopusta
    o->alku = (mahtuu < yht)*(yht - mahtuu); //jos erotus on negatiivinen, kerrotaan 0:lla
  else //laitetaan alusta
    o->alku = -o->rullaus;
  int oy = o->sij.y;
  int raja = (mahtuu < yht)? mahtuu : yht;
  for(int i=0; i<raja; i++) {
    if(o->numerointi) {
      o->teksti = malloc(strlen( sl->taul[o->alku+i] )+10);
      sprintf(o->teksti, "%i. %s", o->alku+1+i, sl->taul[o->alku+i]);
    } else {
      o->teksti = sl->taul[o->alku+i];
    }
    laita_teksti_ttf(o, rend);
    if(o->toteutuma.w > maksw)
      maksw = o->toteutuma.w;
    o->sij.y += rvali;
    if(o->numerointi)
      free(o->teksti);
  }
  o->toteutuma.x = o->sij.x;
  o->toteutuma.y = oy;
  o->toteutuma.w = maksw;
  o->toteutuma.h = o->sij.y - oy;
  o->sij.y = oy;
  return raja;
}

void laita_jarjestus() {
  int rvali = TTF_FontLineSkip(jarjol1.font);
  jarjol1.sij.x = tulosol.toteutuma.x + tulosol.toteutuma.w + 14;
  jarjol1.sij.y = tulosol.toteutuma.y;
  jarjol1.sij.h = (ikkuna_h - jarjol1.sij.y) * jarjsuhde;
  int h_apu = jarjol1.sij.h;
  jarjol1.toteutuma.h = 0;
  int leveystot = 0;
  int i = -jarjol1.rullaus;
  jarjol1.alku = i;
  for(; i<ftulos->pit && h_apu >= rvali; i++) {
    sprintf(jarjol1.teksti, "%i. %s", jarjes[i]+1, stulos->taul[jarjes[i]]);
    laita_teksti_ttf(&jarjol1, rend);
    jarjol1.sij.y += rvali;
    h_apu -= rvali;
    if(jarjol1.toteutuma.w > leveystot)
      leveystot = jarjol1.toteutuma.w;
  }
  jarjol1.toteutuma.h = jarjol1.sij.y - tulosol.toteutuma.y;
  jarjol1.toteutuma.y = tulosol.toteutuma.y;
  jarjol1.toteutuma.w = leveystot;
  /*jarjol2*/
  jarjol2.sij = jarjol1.sij;
  jarjol2.sij.h = ikkuna_h - jarjol2.sij.y;
  int mahtuu = jarjol2.sij.h / rvali;
  int raja = (mahtuu < ftulos->pit-i)? mahtuu : ftulos->pit-i;
  i = ftulos->pit-raja;
  i -= jarjol2.rullaus;
  jarjol2.alku = i;
  for(int j=0; j<raja; j++, i++) {
    sprintf(jarjol2.teksti, "%i. %s", jarjes[i]+1, stulos->taul[jarjes[i]]);
    laita_teksti_ttf(&jarjol2, rend);
    jarjol2.sij.y += rvali;
    if(jarjol2.toteutuma.w > leveystot)
      leveystot = jarjol1.toteutuma.w;
  }
  jarjol2.toteutuma.y = jarjol1.toteutuma.y + jarjol1.toteutuma.h;
  jarjol2.toteutuma.h = rvali*raja;
  jarjol2.toteutuma.w = leveystot;
}

void laita_valinta(vnta_s* o, SDL_Renderer *rend) {
  if(o->valittu)
    SDL_RenderCopy(rend, o->kuvat.valittu, NULL, &o->kuvat.sij);
  else
    SDL_RenderCopy(rend, o->kuvat.ei_valittu, NULL, &o->kuvat.sij);
  laita_teksti_ttf(&(o->teksti), rend);
  return;
}

/*tämä palauttaa toteutumaksi näitten yhteisen alueen*/
void laita_vierekkain(slista* a, slista* b, int alku, tekstiolio_s* o, SDL_Renderer* r) {
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

void laita_oikealle(tekstiolio_s* ov, short vali, slista* l, int alku, tekstiolio_s* o, SDL_Renderer* r) {
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
  if(!o || !o->teksti || !strcmp(o->teksti, ""))
    return;
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
