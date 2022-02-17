#include<alsa/asoundlib.h>
#include<SDL2/SDL.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<sys/time.h>
#include "modkeys.h"
/*
  Tälle annetaan äänidataa ja mahdollisesti sieltä tunnisttut jonkin äänen kohdat.
  Ne piirretään näytölle ja toistetaan ja käyttäjä saa merkitä tunnisteet: oikein tai väärin.
a  Tätä siis käytetään valvottuun oppimiseen
*/
void piirra_raidat();
void aja();
void aanen_valinta(float* data, int raitoja, int raidan_pit, float* kynnysarvot, snd_pcm_t* kahva);
float skaalaa(float* data, int pit);
uint64_t hetkinyt();
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
static SDL_Color kohdistin_paaraita = {0,255,50,255};
static SDL_Color kohdistin_muuraita = {255,80,0,255};
static SDL_Color kynnysvari = {50,180,255,255};
static SDL_Color vntavari = {255,255,255,60};
static struct {int x; int r;} kohdistin = {.x = 0, .r = 0};
static int toiston_x, toiston_alku;
static struct int2 {int a[2];} valinta_x = {{-1, -1}};
static float* data;
static float* kynnysarvot;
static float* skaalat;
static int raitoja, raidan_pit;
static snd_pcm_t* kahva;
static unsigned tuplaklikkaus_ms = 240;
static int toistaa = 0;
static uint64_t hiirihetki0, toistohetki0, hetki=0;

float skaalaa(float* data, int pit) {
  float max = -INFINITY;
  for(int i=0; i<pit; i++)
    if(data[i] > max)
      max = data[i];
    else if(-data[i] > max)
      max = -data[i];
  for(int i=0; i<pit; i++)
    data[i] /= max;
  return max;
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
    skaalat[i] = skaalaa(data+i*raidan_pit, raidan_pit);
  for(int32_t raita=0, y=0; raita<raitoja; raita++, y+=raidan_kork) {
    SDL_Rect alue = {0, y, ikk_w, raidan_h};
    ASETA_VARI(aluevari);
    SDL_RenderFillRect(rend, &alue);
    ASETA_VARI(piirtovari);
    y += raidan_h / 2;
    float* p = data+raita*raidan_pit;
    for(int x=0; x<ikk_w; x++)
      for(int ii=0; ii<ivali; ii++,p++)
	SDL_RenderDrawPoint( rend, x, y-(int)(*p*raidan_h/2) );
    if(kynnysarvot && kynnysarvot[raita]==kynnysarvot[raita]) {
      ASETA_VARI(kynnysvari);
      kynnysarvot[raita]/=skaalat[raita];
      float ytmp = y-(int)(kynnysarvot[raita] * raidan_h/2);
      SDL_RenderDrawLine( rend, 0, ytmp, ikk_w, ytmp );
    }
    y -= raidan_h / 2;
  }
  SDL_SetRenderTarget(rend, NULL);
}

int toiston_sijainti() {
  return toiston_alku + (hetkinyt()-toistohetki0) * 48 / ivali;
}

void piirra_kohdistin(int x, int r) {
  ASETA_VARI(kohdistin_muuraita);
  SDL_RenderDrawLine(rend, x, 0, x, ikk_h);
  ASETA_VARI(kohdistin_paaraita);
  SDL_RenderDrawLine(rend, x, r*raidan_kork, x, r*raidan_kork + raidan_h);
}

void piirra_valinta(struct int2* vnta) {
  if(vnta->a[0] < 0)
    return;
  int pienempi = vnta->a[1] < vnta->a[0];
  static SDL_Rect vntarect;
  vntarect.x = vnta->a[pienempi];
  vntarect.w = vnta->a[!pienempi] - vnta->a[pienempi];
  vntarect.h = ikk_h;
  ASETA_VARI(vntavari);
  SDL_RenderFillRect(rend, &vntarect);
}

void toista_kohdistin() {
  snd_pcm_drop(kahva);
  toistaa = 1;
  toistohetki0 = hetkinyt();
  toiston_alku = kohdistin.x;
  while(snd_pcm_writei( kahva, data+DATAxKOHTA(kohdistin.r, kohdistin.x), raidan_pit-kohdistin.x*ivali ) < 0) {
    snd_pcm_prepare(kahva); //fprintf(stderr, "Puskurin alitäyttö\n");
  }
}

void toista_valinta(struct int2* vnta) {
  snd_pcm_drop(kahva);
  toistaa = 1;
  toistohetki0 = hetkinyt();
  int pienempi = vnta->a[1] < vnta->a[0];
  toiston_alku = vnta->a[pienempi];
  while(snd_pcm_writei( kahva, data+DATAxKOHTA(kohdistin.r, vnta->a[pienempi]), (vnta->a[!pienempi]-vnta->a[pienempi])*ivali ) < 0)
    snd_pcm_prepare(kahva);
}

uint64_t hetkinyt() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
}

void kohdistin_sivulle(int maara) {
  kohdistin.x += maara;
  if(kohdistin.x < 0)
    kohdistin.x = 0;
  else if(kohdistin.x > ikk_w)
    kohdistin.x = ikk_w;
}

void aja() {
  SDL_Event tapaht;
  int siirtoluku = 1;
  unsigned modkey = 0;
 ALKU:
  while(SDL_PollEvent(&tapaht)) {
    switch(tapaht.type) {
    case SDL_QUIT:
      return;
    case SDL_MOUSEBUTTONDOWN:
      if( (hetki=hetkinyt()) - hiirihetki0 < tuplaklikkaus_ms ) {
	toista_kohdistin();
	break;
      }
      hiirihetki0 = hetki;
      kohdistin.x = tapaht.button.x;
      kohdistin.r = tapaht.button.y / raidan_kork;
      break;
    case SDL_KEYDOWN:
      switch(tapaht.key.keysym.scancode) {
      case SDL_SCANCODE_H:
	kohdistin_sivulle(-siirtoluku);
	break;
      case SDL_SCANCODE_L:
	kohdistin_sivulle(siirtoluku);
	break;
      case SDL_SCANCODE_J:
	if(modkey & CTRL)
	  kohdistin.x = 0;
	else
	  kohdistin.r = (kohdistin.r+1) % raitoja;
	break;
      case SDL_SCANCODE_K:
	kohdistin.r = (kohdistin.r-1+raitoja) % raitoja;
	break;
      case SDL_SCANCODE_SEMICOLON:
	if(modkey & CTRL)
	  kohdistin.x = ikk_w;
	break;
      default:
	break;
      } //endswitch scancode
      switch(tapaht.key.keysym.sym) {
#define _MODKEYS_SWITCH_KEYDOWN
#include "modkeys.h"
      case SDLK_SPACE:
	if(modkey & CTRL) {
	  valinta_x.a[0] = valinta_x.a[1] = toistaa? toiston_x : kohdistin.x;
	  break;
	}
	if( (toistaa = (toistaa+1) % 2) ) {
	  if(valinta_x.a[0] < 0)
	    toista_kohdistin();
	  else
	    toista_valinta(&valinta_x);
	}
	else {
	  snd_pcm_drop(kahva);
	  if(modkey & VAIHTO)
	    kohdistin.x = toiston_x;
	}
	break;
      case SDLK_ESCAPE:
	valinta_x.a[0] = -1;
	break;
      default:
	if('1' <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9')
	  siirtoluku = tapaht.key.keysym.sym - '0'; //H- tai L-näppäin siirtää kohdistinta painetun numeron verran
      } //endswitch symcode
      break; //keydown
    case SDL_KEYUP:
      switch(tapaht.key.keysym.sym) {
#define _MODKEYS_SWITCH_KEYUP
#include "modkeys.h"
      default:
	break;
      }
      if('1' <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9')
	siirtoluku = 1;
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
  piirra_kohdistin(kohdistin.x, kohdistin.r);
  if(toistaa) {
    if( (toiston_x = toiston_sijainti()) > (valinta_x.a[0]<0? ikk_w: valinta_x.a[ valinta_x.a[1]>valinta_x.a[0] ]) )
      toistaa = 0;
    piirra_kohdistin(toiston_x, kohdistin.r);
  } else
    valinta_x.a[1] = kohdistin.x;
  piirra_valinta(&valinta_x);
  SDL_RenderPresent(rend);
  SDL_Delay(15);
  goto ALKU;
}

static int oli_sdl = 0;
void aanen_valinta(float* data1, int raitoja1, int raidan_pit1, float* kynnysarvot1, snd_pcm_t* kahva1) {
  raitoja = raitoja1; raidan_pit = raidan_pit1; kahva = kahva1;
  data = malloc(raitoja*raidan_pit*sizeof(float));
  skaalat = malloc(raitoja*sizeof(float));
  memcpy(data, data1, raitoja*raidan_pit*sizeof(float));
  if(kynnysarvot1) {
    kynnysarvot = malloc(raitoja*sizeof(float));
    memcpy(kynnysarvot, kynnysarvot1, raitoja*sizeof(float));
  }
  if(SDL_WasInit(SDL_INIT_VIDEO))
    oli_sdl = 1;
  else
    if(SDL_Init(SDL_INIT_VIDEO)) {
      fprintf(stderr, "Ei alustettu SDL-grafiikkaa: %s\n", SDL_GetError());
      return;
    }
  SDL_DisplayMode dm;
  if(SDL_GetCurrentDisplayMode(0, &dm))
    fprintf(stderr, "Virhe näytön koon tiedoissa (äänen_valinta):\n%s\n", SDL_GetError());
  ikk_w = dm.w;
  ikk_h = raidan_kork * raitoja;
  ikk_x0 = 0;
  ikk_y0 = 0;
  ikkuna = SDL_CreateWindow("Äänen valinta", ikk_x0, ikk_y0, ikk_w, ikk_h, SDL_WINDOW_RESIZABLE);
  rend = SDL_CreateRenderer(ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);
  SDL_GetWindowSize(ikkuna, &ikk_w, &ikk_h); //ikkunointimanageri voi muuttaa kokoa pyydetystä
  tausta = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ikk_w, ikk_h);
  SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);

  piirra_raidat();
  aja();

  free(data); data=NULL;
  free(skaalat); skaalat=NULL;
  if(kynnysarvot1) { free(kynnysarvot); kynnysarvot=NULL; }
  SDL_DestroyTexture(tausta);
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(ikkuna);
  if(!oli_sdl)
    SDL_Quit();
}
