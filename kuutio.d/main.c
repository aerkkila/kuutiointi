#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdarg.h>
#include "liity_muistiin.h"

#include "kuutio.c"
#include "kuution_grafiikka.c"

kuutio_t kuutio;

#ifndef __EI_SEKUNTIKELLOA__
#include "liity_muistiin.c"
#include "lue_siirrot.c"
#endif

#ifndef EI_SAVEL_MAKRO
#include "python_savel.c"
#endif

int main(int argc, char** argv) {
  int N;
  if( argc < 2 || !(sscanf(argv[1], "%i", &N)) )
    N = 3;
  
  kuutio = luo_kuutio(N);
  
#ifndef __EI_SEKUNTIKELLOA__
  ipc = liity_muistiin();
  if(!ipc)
    return 1;
#endif

  char oli_sdl = 0;
  if(!SDL_WasInit(SDL_INIT_VIDEO))
    SDL_Init(SDL_INIT_VIDEO);
  else
    oli_sdl = 1;

  if(luo_kuva())
    goto ULOS;
  tee_ruutujen_koordtit();
  kaantoaika = kaantoaika0;
  for(int i=0; i<2; i++) {
    alusta[i] = SDL_CreateTexture(kuva.rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, kuva.xRes, kuva.yRes);
    SDL_SetTextureBlendMode(alusta[i], SDL_BLENDMODE_BLEND); //alustan kopioinnissa on alfa-kanava
    if(!alusta[i])
      return 1;
  }
  SDL_SetRenderDrawBlendMode(kuva.rend, SDL_BLENDMODE_NONE); //muualla otetaan sellaisenaan
#ifdef AUTOMAATTI
#include "automaattikuutio.c" //toistetaan jotain sarjaa automaattisesti
#endif
  
#include "kuution_käyttöliittymä.c"
  
 ULOS:
#ifndef EI_SAVEL_MAKRO
  if(savelPtr) {
    if(system("pkill sävel.py") < 0)
      printf("Sävel-ohjelmaa ei suljettu\n");
    savelPtr = NULL;
  }
#endif
  for(int i=0; i<2; i++)
    SDL_DestroyTexture(alusta[i]);
  SDL_DestroyRenderer(kuva.rend);
  SDL_DestroyWindow(kuva.ikkuna);
  free(kuva.ruudut);
  if(!oli_sdl)
    SDL_Quit();
  free(kuutio.sivut);
  return 0;
}
