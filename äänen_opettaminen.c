#include<alsa/asoundlib.h>
#include<SDL2/SDL.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<sys/time.h>
/*
  Tälle annetaan äänidataa ja mahdollisesti sieltä tunnisttut jonkin äänen kohdat.
  Ne piirretään näytölle ja toistetaan ja käyttäjä saa merkitä tunnisteet: oikein tai väärin.
  Tätä siis käytetään valvottuun oppimiseen
*/
void piirra_raidat();
void aja();
void aanen_opettaminen(float* data, int raitoja, int raidan_pit, snd_pcm_t* kahva);
void skaalaa(float* data, int pit);
#define ASETA_VARI(vari) SDL_SetRenderDrawColor(rend, vari.r, vari.g, vari.b, vari.a)
#define DATAxKOHTA(raita,xkohta) ((raita)*raidan_pit + (xkohta)*ivali)

static int ikk_x0=0, ikk_y0=0, ikk_w, ikk_h; //w on ikkunan leveys, h riippuu raitojen määrästä
static int32_t valin_suhde = 14, raitoja, raidan_pit;
static int raidan_kork = 200, raidan_vali, raidan_h, ivali;
static SDL_Window* ikkuna;
static SDL_Renderer* rend;
static SDL_Texture* tausta;
static SDL_Color taustavari = {40,40,40,255};
static SDL_Color aluevari = {.a=255};
static SDL_Color piirtovari = {255,255,255,255};
static SDL_Color kohdistin_paaraita = {255,80,0,255};
static SDL_Color kohdistin_muuraita = {0,255,50,255};
static struct {int x; int r;} kohdistin = {.x = 0, .r = 0};
static float* data;
static int raitoja, raidan_pit;
static snd_pcm_t* kahva;
static unsigned tuplaklikkaus_ms = 240;
static int toistaa = 0;

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
  ivali = raidan_pit/ikk_w;
  raidan_vali = ikk_h / (raitoja*valin_suhde - 1); //saadaan ratkaisemalla yhtälöpari kynällä ja paperilla
  raidan_kork = ikk_h * valin_suhde / (raitoja*valin_suhde - 1);
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
    for(int x=0; x<ikk_w; x++) //oletetaan sama määrä epälukuja olevan myös loppupäässä
      for(int ii=0; ii<ivali; ii++,p++)
	SDL_RenderDrawPoint( rend, x, y-(int)(*p*raidan_h/2) );
    y -= raidan_h / 2;
  }
  SDL_SetRenderTarget(rend, NULL);
}

void piirra_kohdistin() {
  ASETA_VARI(kohdistin_muuraita);
  SDL_RenderDrawLine(rend, kohdistin.x, 0, kohdistin.x, ikk_h);
  ASETA_VARI(kohdistin_paaraita);
  SDL_RenderDrawLine(rend, kohdistin.x, kohdistin.r*raidan_kork, kohdistin.x, kohdistin.r*raidan_kork + raidan_h);
}

void toista_kohdistin() {
  while(snd_pcm_writei( kahva, data+DATAxKOHTA(kohdistin.r, kohdistin.x), raidan_pit-kohdistin.x*ivali ) < 0) {
    snd_pcm_prepare(kahva);
    fprintf(stderr, "Puskurin alitäyttö\n");
  }
  fflush(stdout);
}

uint64_t hetkinyt() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
}

void aja() {
  SDL_Event tapaht;
  uint64_t hetki0=0, hetki=0;
 ALKU:
  while(SDL_PollEvent(&tapaht)) {
    switch(tapaht.type) {
    case SDL_QUIT:
      return;
    case SDL_MOUSEBUTTONDOWN:
      if( (hetki=hetkinyt()) - hetki0 < tuplaklikkaus_ms ) {
	toista_kohdistin();
	break;
      }
      hetki0 = hetki;
      kohdistin.x = tapaht.button.x;
      kohdistin.r = tapaht.button.y / raidan_kork;
      break;
    case SDL_KEYDOWN:
      if( tapaht.key.keysym.sym == SDLK_SPACE ) {
	if( (toistaa = (toistaa+1) % 2) )
	  toista_kohdistin();
	else 
	  snd_pcm_drop(kahva);
      }
      break;
    case SDL_WINDOWEVENT:
      if( tapaht.window.event != SDL_WINDOWEVENT_RESIZED)
	break;
      SDL_GetWindowSize(ikkuna, &ikk_w, &ikk_h);
      SDL_DestroyTexture(tausta);
      tausta = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ikk_w, ikk_h);
      piirra_raidat();
    }
  }
  SDL_RenderCopy(rend, tausta, NULL, NULL);
  piirra_kohdistin();
  SDL_RenderPresent(rend);
  SDL_Delay(15);
  goto ALKU;
}

static int oli_sdl = 0;
void aanen_opettaminen(float* data1, int raitoja1, int raidan_pit1, snd_pcm_t* kahva1) {
  raitoja = raitoja1; raidan_pit = raidan_pit1; kahva = kahva1;
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
