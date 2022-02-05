#include <stdio.h>
#include <SDL2/SDL.h>
#include "asetelma.h"
#include "grafiikka.h"
#include "tulokset.h"

#define PYYHI(olio) SDL_RenderFillRect(rend, &olio.toteutuma)
#define KELLO (kellool.teksti)

/*Muuttuja 'laitot' määrittää, mitkä osat piirretään uudestaan.
Jos laitot == jaaduta, koko kuva piirretään 'tausta'-tekstuuriin
Jos laitot == 0, tausta vain kopioidaan näytölle.
Jos laitot == kaikki_laitot, koko kuva paitsi kello piirretäan 'tausta'-tekstuuriin
Jos laitot == kellolai, tausta kopioidaan näytölle ja laitetaan kello päälle.
Jos laitot & JOTAINlai, kyseinen JOTAIN pyyhitään taustasta ja piirretään uudestaan
Käyttöliittymässä asetettakoon tämän jälkeen aina laitot = kellolai * (tila != seis)*/

void laita_jarjestus();
unsigned short laitot = 0x07ff;
const unsigned short kellolai  = 0x0001;
const unsigned short sektuslai = 0x0002;
const unsigned short tuloslai  = 0x0004;
const unsigned short jarjlai   = 0x0008;
const unsigned short tiedtlai  = 0x0010;
const unsigned short lisatdlai = 0x0020;
const unsigned short muutlai   = 0x0040;
const unsigned short tkstallai = 0x0080;
const unsigned short vntalai   = 0x0100;
const unsigned short kaikki_laitot = 0x01ff;
const unsigned short jaaduta   = 0x03ff; //myös kello laitetaan taustaan

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

void saada_kohdistin() {
  kohdistinsij.y = kellool.sij.y;
  kohdistinsij.h = TTF_FontLineSkip(kellool.font);
  TTF_GlyphMetrics(kellool.font, ' ', NULL, NULL, NULL, NULL, &kohdistinsij.w);
  kohdistinsij.w /= 10;
  if(!kohdistinsij.w)
    kohdistinsij.w = 1;
}

void aseta_vari(SDL_Renderer* rend, SDL_Color* vari) {
  SDL_SetRenderDrawColor(rend, vari->g, vari->b, vari->b, vari->a);
}

void laita_ehka_kohdistin() {
  if(kohdistin >= 0) {
    saada_kohdistin();
    aseta_vari(rend, &kohdistinvari);
    kohdistinsij.x = xsijainti(&kellool, strlen(kellool.teksti)-kohdistin);
    SDL_RenderFillRect(rend, &kohdistinsij);
  }
}

void piirra() {
  if(laitot == kellolai) {
    SDL_RenderCopy(rend, tausta, NULL, NULL);
    if(KELLO[0])
      laita_teksti_ttf(&kellool, rend);
    laita_ehka_kohdistin();
    laita_ehka_korostus();
    SDL_RenderPresent(rend);
    return;
  }
  if(!laitot) {
    SDL_RenderCopy(rend, tausta, NULL, NULL);
    laita_ehka_korostus();
    SDL_RenderPresent(rend);
    return;
  }
  SDL_SetRenderTarget(rend, tausta);
  aseta_vari(rend, &taustavari);
  if( (laitot & kaikki_laitot) == kaikki_laitot ) {
    SDL_RenderClear(rend);
    goto LAITOT;
  }
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

 LAITOT:
  if(laitot & vntalai) {
    laita_valinta(&tarknap, rend);
  }
  if(laitot & muutlai) {
    laita_tekstilista(muut_a, 0, &muutol, rend);
  }
  if(laitot & sektuslai) {
    sektusol.sij.h = ikkuna_h - sektusol.sij.y;
    laita_tekstilista(sektus, 1, &sektusol, rend);
  }
  if(laitot & tuloslai) {
    tulosol.sij.h = ikkuna_h - tulosol.sij.y;
    laita_oikealle(&kellool, 10, stulos, 1, &tulosol, rend);
  }
  if(laitot & tkstallai) {
    laita_teksti_ttf_vasemmalle(&tulosol, 10, &tkstalol, rend);
  }
  if(laitot & jarjlai) {
    laita_jarjestus();
  }
  if(laitot & tiedtlai) {
    /*tässä muuttujien nimet ovat aivan epäloogiset*/
    laita_oikealle(&jarjol1, 25, tietoalut, 1, &tiedotol, rend);
    laita_oikealle(&tiedotol, 0, tietoloput, 1, &tluvutol, rend);
  }
  if(laitot & lisatdlai) {
    lisaol.sij.h = ikkuna_h - lisaol.sij.y;
    laita_oikealle(&jarjol1, 20, lisatd, 0, &lisaol, rend);
  }
  /*Kelloa ei laiteta taustaan ellei jäädytetä. Tässä se laitetaan suoraan näytölle.*/
  if(laitot != jaaduta) {
    SDL_SetRenderTarget(rend, NULL);
    SDL_RenderCopy(rend, tausta, NULL, NULL);
    if(KELLO[0]) //ei tarkisteta, onko kelloa käsketty laittaa, 0:sta on palattu jo aiemmin
      laita_teksti_ttf(&kellool, rend);
    laita_ehka_kohdistin();
    laita_ehka_korostus();
    SDL_RenderPresent(rend);
    return;
  }
  if(KELLO[0]) //jäädytä
    laita_teksti_ttf(&kellool, rend);
  laita_ehka_kohdistin(); //tuskin laitetaan
  SDL_SetRenderTarget(rend, NULL);
  SDL_RenderCopy(rend, tausta, NULL, NULL);
  laita_ehka_korostus();
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
  int yht = sl->pit;
  /*tässä toteutumaksi tulee maksimit*/
  int maksw = 0;
  
  /*laitetaan niin monta jäsentä kuin mahtuu*/
  if(alku) //laitetaan lopusta
    o->alku = (mahtuu < yht)*(yht - mahtuu - o->rullaus); //0, jos mahtuu >= yht
  else //laitetaan alusta
    o->alku = -o->rullaus;
  int oy = o->sij.y;
  int raja = (mahtuu < yht-o->alku)? mahtuu : yht-o->alku;
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
      leveystot = jarjol2.toteutuma.w;
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

void laita_ehka_korostus() { //jos korostusol.paksuus <= 0, ei laiteta
  aseta_vari(rend, &korostusol.vari);
  SDL_Rect apu = korostusol.kulmio;
  for(int i=0; i<korostusol.paksuus; i++) {
    SDL_RenderDrawRect(rend, &apu);
    apu.x--;
    apu.y--;
    apu.w+=2;
    apu.h+=2;
  }
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
