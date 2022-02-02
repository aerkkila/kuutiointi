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
void piirra_raidat();
void aja();
void aanen_opettaminen(float* data, int raitoja, int raidan_pit);
void skaalaa(float* data, int pit);
#define ASETA_VARI(vari) SDL_SetRenderDrawColor(rend, vari.r, vari.g, vari.b, vari.a)

static int ikk_x0=0, ikk_y0=0, ikk_w, ikk_h; //w on ikkunan leveys, h riippuu raitojen määrästä
static int32_t valin_suhde = 14, raitoja, raidan_pit;
static int raidan_kork = 200, raidan_vali, raidan_h;
static SDL_Window* ikkuna;
static SDL_Renderer* rend;
static SDL_Texture* tausta;
static SDL_Color taustavari = {40,40,40,255};
static SDL_Color aluevari = {.a=255};
static SDL_Color piirtovari = {255,255,255,255};
static int kohdistin;
static float* data;
static int raitoja, raidan_pit;

void skaalaa(float* data, int pit) {
  float max = -INFINITY;
  for(int i=0; i<pit; i++)
    if(data[i] > max)
      max = data[i];
    else if(-data[i] > max)
      max = -data[i];
  for(int i=0; i<pit; i++)
    data[i] /= max;
}

void piirra_raidat() {
  int ivali = raidan_pit/ikk_w;
  raidan_kork = ikk_h / raitoja;
  raidan_vali = raidan_kork / valin_suhde;
  raidan_h = raidan_kork - raidan_vali;
  SDL_SetRenderTarget(rend, tausta);
  ASETA_VARI(taustavari);
  SDL_RenderClear(rend);
  for(int i=0; i<raitoja; i++)
    skaalaa(data+i*raidan_pit, raidan_pit);
  for(int32_t raita=0, y=0; raita<raitoja; raita++, y+=raidan_kork) {
    SDL_Rect alue = {0, y, ikk_w, raidan_h};
    ASETA_VARI(aluevari);
    SDL_RenderFillRect(rend, &alue);
    ASETA_VARI(piirtovari);
    y += raidan_h / 2;
    float* p = data+raita*raidan_pit;
    SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
    SDL_RenderDrawLine(rend, 500,y-raidan_h/2, 500,y+raidan_h/2);
    ASETA_VARI(piirtovari);
    int apu = 0;
    while(p[apu] != p[apu]) //ohitetaan epäluvut
      apu++;
    int x0 = apu/ivali + !!(apu%ivali); //ceil(apu/ivali)
    p += x0*ivali;
    for(int x=x0; x<ikk_w-x0; x++) //oletetaan sama määrä epälukuja myös loppupäähän
      for(int ii=0; ii<ivali; ii++,p++)
	SDL_RenderDrawPoint( rend, x, y-(int)(*p*raidan_h/2) );
    y -= raidan_h / 2;
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

static int oli_sdl = 0;
void aanen_opettaminen(float* data1, int raitoja1, int raidan_pit1) {
  raitoja = raitoja1; raidan_pit = raidan_pit1;
  data = malloc(raitoja*raidan_pit*sizeof(float));
  memcpy(data, data1, raitoja*raidan_pit*sizeof(float));
  if(SDL_WasInit(SDL_INIT_VIDEO))
    oli_sdl = 1;
  else
    if(SDL_Init(SDL_INIT_VIDEO)) {
      fprintf(stderr, "Ei alustettu SDL-grafiikkaa: %s\n", SDL_GetError());
      return;
    }
  SDL_DisplayMode dm;
  if(SDL_GetCurrentDisplayMode(0, &dm))
    fprintf(stderr, "Virhe näytön koon tiedoissa (äänen_opettaminen):\n%s\n", SDL_GetError());
  ikk_w = dm.w;
  ikk_h = raidan_kork * raitoja;
  ikk_x0 = 0;
  ikk_y0 = 0;
  ikkuna = SDL_CreateWindow("Äänen opettaminen", ikk_x0, ikk_y0, ikk_w, ikk_h, SDL_WINDOW_RESIZABLE);
  rend = SDL_CreateRenderer(ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);
  SDL_GetWindowSize(ikkuna, &ikk_w, &ikk_h); //ikkunointimanageri voi muuttaa kokoa pyydetystä
  tausta = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ikk_w, ikk_h);

  piirra_raidat();
  aja();

  free(data); data=NULL;
  SDL_DestroyTexture(tausta);
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(ikkuna);
  if(!oli_sdl)
    SDL_Quit();
}
