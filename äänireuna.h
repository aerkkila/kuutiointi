#ifndef __AANIREUNA_H__
#define __AANIREUNA_H__
enum {
  aanireuna_opettaminen=1,
};

int tallennusaika_ms = 5000;
uint32_t opetusviive_ms = -1;
int p00=-1, p01=-1, p10=-1, p11=-1;

const struct {char* nimi; char* muoto; void* muuttujat[3];} kntoarg[] = {
  { "--tallennusaika_ms", "%i", {&tallennusaika_ms} },
  { "--opetusviive_ms", "%i", {&opetusviive_ms} },
  { "--putki0", "%i", {&p00, &p01} },
  { "--putki1", "%i", {&p10, &p11} },
};
#endif
