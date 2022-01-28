#include<alsa/asoundlib.h>
#include<SDL2/SDL.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
/*
  Tälle annetaan äänidataa ja mahdollisesti sieltä tunnisttut jonkin äänen kohdat.
  Ne piirretään näytölle ja toistetaan ja käyttäjä saa merkitä tunnisteet: oikein tai väärin.
  Tätä siis käytetään valvottuun oppimiseen

  Putkessa annetaan ensin raitojen määrä (int32_t), ja pituus (int32_t). Kaikilla on sama pituus.
  Sitten aina float32 monoääni
*/
void piirra();
#define ASETA_VARI(vari) SDL_SetRenderDrawColor(rend, vari.r, vari.g, vari.b, vari.a)

int ikk_x0=0, ikk_y0=0, ikk_w, ikk_h; //w on ikkunan leveys, h riippuu raitojen määrästä
int32_t valin_suhde = 14, raitoja, raidan_pit;
int raidan_kork = 140, raidan_vali, raidan_h;
SDL_Window* ikkuna;
SDL_Renderer* rend;
SDL_Texture* tausta;
SDL_Color taustavari = {40,40,40,255};
SDL_Color aluevari = {.a=255};
SDL_Color piirtovari = {255,255,255,255};
int luku, kirj, apuint, kohdistin;
float* aanet;

void piirra_raidat() {
  float* p = aanet;
  int ivali = raidan_pit/ikk_w;
  raidan_kork = ikk_h / raitoja;
  raidan_vali = raidan_kork / valin_suhde;
  raidan_h = raidan_kork - raidan_vali;
  SDL_SetRenderTarget(rend, tausta);
  ASETA_VARI(taustavari);
  SDL_RenderClear(rend);
  for(int32_t raita=0, y=0; raita<raitoja; raita++, y+=raidan_kork) {
    SDL_Rect alue = {0, y, ikk_w, raidan_h};
    ASETA_VARI(aluevari);
    SDL_RenderFillRect(rend, &alue);
    ASETA_VARI(piirtovari);
    for(int x=0; x<ikk_w; x++)
      for(int ii=0; ii<ivali; ii++,p++)
	SDL_RenderDrawPoint(rend, x, y - *p*raidan_h);
  }
  SDL_SetRenderTarget(rend, NULL);
}

void aja() {
  SDL_Event tapaht;
 ALKU:
  while(SDL_PollEvent(&tapaht)) {
    switch(tapaht.type) {
    case SDL_QUIT:
      return;
    case SDL_MOUSEBUTTONDOWN:
      kohdistin = tapaht.button.x;
      break;
    case SDL_WINDOWEVENT:
      if( tapaht.window.event != SDL_WINDOWEVENT_RESIZED)
	break;
      SDL_GetWindowSize(ikkuna, &ikk_w, &ikk_h);
      piirra_raidat();
    }
  }
  SDL_RenderCopy(rend, tausta, NULL, NULL);
  SDL_RenderPresent(rend);
  SDL_Delay(15);
  goto ALKU;
}

int main(int argc, char** argv) {
  for(int i=1; i<argc; i++) {
    if(!strcmp(argv[i], "--putki1")) {
      if(argc <= i+2 || sscanf(argv[i+1], "%i", &kirj)!=1 || sscanf(argv[i+2], "%i", &apuint)!=1) {
	fprintf(stderr, "Ei putkea argumentin --putki1 jälkeen\n");
	continue;
      }
      close(apuint);
      i+=2;
    }
    if(!strcmp(argv[i], "--putki0")) {
      if(argc <= i+2 || sscanf(argv[i+1], "%i", &luku)!=1 || sscanf(argv[i+2], "%i", &apuint)!=1) {
	fprintf(stderr, "Ei putkea argumentin --putki0 jälkeen\n");
	continue;
      }
      close(apuint);
      i+=2;
    }
  }
  read(luku, &raitoja, 4);
  read(luku, &raidan_pit, 4);
  aanet = malloc(raitoja*raidan_pit*sizeof(float*));
  read(luku, aanet, raitoja*raidan_pit);
  close(luku);
  SDL_DisplayMode dm;
  SDL_GetCurrentDisplayMode(0, &dm);
  ikk_w = dm.w;
  ikk_h = raidan_kork * raitoja - raidan_kork / valin_suhde;
  ikk_x0 = 0;
  ikk_y0 = 0;
  if(!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "Ei alustettu SDL-grafiikkaa: %s\n", SDL_GetError());
    return 1;
  }
  ikkuna = SDL_CreateWindow("Äänen opettaminen", ikk_x0, ikk_y0, ikk_w, ikk_h, SDL_WINDOW_RESIZABLE);
  rend = SDL_CreateRenderer(ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);
  SDL_GetWindowSize(ikkuna, &ikk_w, &ikk_h); //ikkunointimanageri voi muuttaa kokoa pyydetystä
  tausta = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ikk_w, ikk_h);

  piirra_raidat();
  aja();

  free(aanet); aanet=NULL;
  SDL_DestroyTexture(tausta);
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(ikkuna);
  SDL_Quit();
  return 0;
}
