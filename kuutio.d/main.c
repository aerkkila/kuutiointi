#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <errno.h>
#include "../muistin_jako.h"

#include "kuutio.c"
#include "kuution_grafiikka.c"

kuutio_t kuutio;

#ifndef __EI_SEKUNTIKELLOA__
#include "lue_siirrot.c"
#endif

#ifndef EI_SAVEL_MAKRO
#include "python_savel.c"
#endif

/* funktio kopioitu nctietue2-kirjastosta */
static int get_modstate() {
  /* makes modstate side-insensitive and removes other modifiers than [alt,ctrl,gui,shift] */
  int mod = 0;
  int mod0 = SDL_GetModState();
  if(mod0 & KMOD_CTRL)
    mod |= KMOD_CTRL;
  if(mod0 & KMOD_SHIFT)
    mod |= KMOD_SHIFT;
  if(mod0 & KMOD_ALT)
    mod |= KMOD_ALT;
  if(mod0 & KMOD_GUI)
    mod |= KMOD_GUI;
  return mod;
}

void avaa_tallenne(const char* s) {
  char a[256];
  if(!s) {
    sprintf(a, "%s/.cache/kuution_tallenne.bin", getenv("HOME"));
    s = a;
  }
  struct stat buf;
  if(stat(s, &buf) < 0) {
    perror("tallenteen avaaminen");
    kuutio = luo_kuutio(3);
    return;
  }
  FILE *f = fopen(s, "r");
  int N2 = buf.st_size / 6, N=0;
  /* Alla oleva rivi laskee neliöjuuren neliölukujen avulla. Menetelmä löytyy Wikipediasta. */
  for(int v=-1; N2-v>=2; N2-=v+=2) N++;
  kuutio = luo_kuutio(N);
  if(fread(kuutio.sivut, 1, N*N*6, f) != N*N*6)
    perror("tallenteen luenta");
  fclose(f);
}

int main(int argc, char** argv) {
  int N;
  if(argc < 2 || !(sscanf(argv[1], "%i", &N)))
    N = 3;
  if(argc > 1)
    if(!strcmp(argv[1], "s"))
      avaa_tallenne(NULL);
    else
      avaa_tallenne(argv[1]);
  else
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
    if(system("pkill savel.py") < 0)
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
