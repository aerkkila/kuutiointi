#ifndef __grafiikka__
#define __grafiikka__

#include <listat.h>
#include "rakenteet.h"

inline laitot_s __attribute__((always_inline)) kaikki_laitot() {
  return (laitot_s){1, 1, 1, 1, 1, 1, 1, 1};
}
void piirra(kaikki_s*);
void laita_teksti_ttf(tekstiolio_s*, SDL_Renderer*);
void laita_tekstilista(strlista*, int, tekstiolio_s*, SDL_Renderer*);
void laita_tekstilistan_paat(strlista*, tekstiolio_s*, float, SDL_Renderer*);
void laita_aaret(tekstiolio_s* ov, short vali, strlista* luvut, strlista* l, tekstiolio_s*, float, SDL_Renderer*);
void laita_valinta(vnta_s* o, SDL_Renderer *rend);
void laita_tiedot(strlista* a, tekstiolio_s* oa,			\
		  strlista* b, tekstiolio_s* ob, SDL_Renderer* r);
void laita_vierekkain(strlista* a, strlista* b, int alku, tekstiolio_s* o, SDL_Renderer* r);
void laita_vasemmalle(tekstiolio_s* ov, short vali, strlista* l, int alku, tekstiolio_s* o, SDL_Renderer* r);

#endif
