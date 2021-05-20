#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <listat.h>
#include <tekstigraf.h>
#include "cfg.h"
#include "grafiikka.h"

#define PYYHI(olio) SDL_RenderFillRect(rend, olio.toteutuma)
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

extern float skaala;

void piirra() {
  /*pyyhitään vanhat*/
  if(laitot & kellolai)
    PYYHI(kellool);
  if(laitot & vntalai) {
    SDL_RenderFillRect(rend, vntaol.kuvat->sij);
    PYYHI(vntaol.teksti);
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
    laita_valinta(&vntaol, rend);
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
    laita_oikealle(&kellool, 10, _yalkuun(tkset.strtulos), 1, &tulosol, rend);
    laitot &= ~tuloslai;
  }
  if(laitot & tkstallai) {
    laita_teksti_ttf_vasemmalle(&tulosol, 10, &tkstalol, rend);
    laitot &= ~tkstallai;
  }
  if(laitot & jarjlai) {
    strlista* a = _ynouda(_yalkuun(tkset.sijarj), 1);
    strlista* b = _ynouda(_yalkuun(tkset.strjarj), 1);
    int n = laita_pari_oikealle(&tulosol, 20, a, b, 0, &jarjol1, rend);
    a = _ynouda(a, n);
    b = _ynouda(b, n);
    laita_pari_oikealle(&tulosol, 20, a, b, 1, &jarjol2, rend);
    jarjol2.alku += n; //listaa ei ollut annettu alusta asti
    if(jarjol1.toteutuma->w < jarjol2.toteutuma->w)
      jarjol1.toteutuma->w = jarjol2.toteutuma->w;
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

void laita_valinta(vnta_s* o, SDL_Renderer *rend) {
  if(o->valittu)
    SDL_RenderCopy(rend, o->kuvat->valittu, NULL, o->kuvat->sij);
  else
    SDL_RenderCopy(rend, o->kuvat->ei_valittu, NULL, o->kuvat->sij);
  laita_teksti_ttf(&(o->teksti), rend);
  return;
}
