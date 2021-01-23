#ifndef __grafiikka__
#define __grafiikka__

#include <listat.h>
#include "rakenteet.h"

inline laitot_s __attribute__((always_inline)) kaikki_laitot() {
  return (laitot_s){1, 1, 1, 1, 1, 1, 1, 1, 1};
}

void piirra(kaikki_s*);
void laita_teksti_ttf(tekstiolio_s*, SDL_Renderer*);
int laita_tekstilista(strlista*, int, tekstiolio_s*, SDL_Renderer*);
int laita_pari_oikealle(tekstiolio_s* ov, int vali,		\
			strlista* a, strlista* b, int alku,	\
			tekstiolio_s* o, SDL_Renderer* rend);
void laita_valinta(vnta_s* o, SDL_Renderer *rend);
void laita_tiedot(strlista* a, tekstiolio_s* oa,			\
		  strlista* b, tekstiolio_s* ob, SDL_Renderer* r);
void laita_vierekkain(strlista* a, strlista* b, int alku, tekstiolio_s* o, SDL_Renderer* r);
void laita_oikealle(tekstiolio_s* ov, short vali, strlista* l, int alku, tekstiolio_s* o, SDL_Renderer* r);
void laita_teksti_ttf_vasemmalle(tekstiolio_s* ov, short vali, tekstiolio_s* o, SDL_Renderer* r);

#endif
