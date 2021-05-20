#ifndef __grafiikka__
#define __grafiikka__

#include <listat.h>
#include <tekstigraf.h>
#include "cfg.h"

#endif

extern unsigned short laitot;
extern const unsigned short kellolai;
extern const unsigned short vntalai;
extern const unsigned short sektuslai;
extern const unsigned short tuloslai;
extern const unsigned short jarjlai;
extern const unsigned short tiedtlai;
extern const unsigned short lisatdlai;
extern const unsigned short muutlai;
extern const unsigned short tkstallai;
extern const unsigned short muuta_tulos;
extern const unsigned short kaikki_laitot;

void piirra();
void laita_valinta(vnta_s* o, SDL_Renderer *rend);
