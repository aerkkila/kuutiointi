#include <alsa/asoundlib.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "modkeys.h"
#include "äänireuna.h"

typedef union {
  int i;
  float f;
  void* v;
} Arg;

typedef struct {
  int tapahtuma;
  unsigned mod;
  void (*fun)(Arg);
  Arg arg;
} Sidonta;

struct int2 {int a[2];};

void napp_alas(Arg turha);
void napp_ylos(Arg turha);
void hiiri_alas(Arg turha);
void ikkunatapahtuma(Arg turha);
void kohdistin_sivulle(Arg i_suunta);
void kohdistin_alas(Arg i_suunta);
void kuvan_alku_sivulle(Arg i_suunta);
void zoomaa(Arg f_suunta);
void vaihda_toistaminen(Arg vp_uusi_kohdistin);
void alusta_valinta(Arg vp_kohta);
void laita_unixaika(Arg vp_xkohta);
void laita_aika(Arg vp_xkohta);
void int_nollaksi(Arg vp_muuttuja);

void piirra_raidat();
void piirra_kohdistin(int x, int r);
void piirra_valinta(struct int2*);
int toiston_sijainti();
void toista_kohdistin();
void toista_vali(struct int2);
uint64_t hetkinyt();
float skaalaa(float* data, int pit);

void aja();
void aanen_valinta(float* data, int raitoja, int raidan_pit, float* kynnysarvot, snd_pcm_t* kahva, int ulos_fno);
#define ASETA_VARI(vari) SDL_SetRenderDrawColor(rend, vari.r, vari.g, vari.b, vari.a)
#define DATAxKOHTA(raita,xkohta) ((raita)*raidan_pit + (xkohta)*ivali)
#define LASKE_IVALI ( (int)( raidan_pit/ikk_w / zoom ) )
#define PITUUS(taul) (sizeof(taul)/sizeof(*taul))

extern uint64_t aanen_loppuhetki;
static SDL_Window* ikkuna;
static SDL_Renderer* rend;
static SDL_Texture* tausta;
static SDL_Event tapaht;
/*asetelma*/
static SDL_Color taustavari = {40,40,40,255};
static SDL_Color aluevari = {.a=255};
static SDL_Color piirtovari = {255,255,255,255};
static SDL_Color kohdistin_paaraita = {0,255,50,255};
static SDL_Color kohdistin_muuraita = {255,80,0,255};
static SDL_Color kynnysvari = {50,180,255,255};
static SDL_Color vntavari = {255,255,255,60};
static int ikk_x0=0, ikk_y0=0, ikk_w, ikk_h; //ikk_w on näytön leveys, ikk_h riippuu raitojen määrästä
static int32_t valin_suhde = 14, raitoja, raidan_pit;
static int raidan_kork = 200, raidan_vali, raidan_h, ivali, kuvan_alku_x;
static float zoom = 1;
static unsigned tuplaklikkaus_ms = 240, siirtoluku = 1;

static struct {int x; int r;} kohdistin = {.x = 0, .r = 0}; //x-sijainti raidan, ei kuvan mukaan
static int toiston_x, toiston_alku;
static struct int2 valinta_x = {{-1, -1}};
static float* data;
static float* kynnysarvot;
static float* skaalat;
static int raitoja, raidan_pit;
static snd_pcm_t* kahva;
static int ulos_fno;
static int toistaa = 0, piirto_raidat=0, jatka=1;
static uint64_t hiirihetki0, toistohetki0, hetki=0;
static unsigned modkey;

Sidonta napp_alas_sid[] = {
  { SDLK_g,        0,      kohdistin_sivulle,  {.i=-1}           },
  { SDLK_o,        0,      kohdistin_sivulle,  {.i=1}            },
  { SDLK_a,        0,      kohdistin_alas,     {.i=1}            },
  { SDLK_i,        0,      kohdistin_alas,     {.i=-1}           },
  { SDLK_g,        ALT,    kuvan_alku_sivulle, {.i=-1}           },
  { SDLK_o,        ALT,    kuvan_alku_sivulle, {.i=1}            },
  { SDLK_SPACE,    0,      vaihda_toistaminen, {0}               },
  { SDLK_SPACE,    ALT,    vaihda_toistaminen, {.v=&toiston_x}   },
  { SDLK_SPACE,    CTRL,   alusta_valinta,     {.v=&kohdistin.x} },
  { SDLK_ESCAPE,   0,      alusta_valinta,     {0}               },
  { SDLK_PLUS,     0,      zoomaa,             {.f=1.1}          },
  { SDLK_KP_PLUS,  0,      zoomaa,             {.f=1.1}          },
  { SDLK_MINUS,    0,      zoomaa,             {.f=1/1.1}        },
  { SDLK_KP_MINUS, 0,      zoomaa,             {.f=1/1.1}        },
  { SDLK_RETURN,   0,      laita_unixaika,     {.v=&kohdistin.x} },
  { SDLK_KP_ENTER, 0,      laita_unixaika,     {.v=&kohdistin.x} },
  { SDLK_RETURN,   ALT,    laita_aika,         {.v=&kohdistin.x} },
  { SDLK_KP_ENTER, ALT,    laita_aika,         {.v=&kohdistin.x} },
};

Sidonta tapaht_sid[] = {
  { SDL_QUIT,            0, int_nollaksi,    {.v=&jatka} },
  { SDL_KEYDOWN,         0, napp_alas,       {0}         },
  { SDL_KEYUP,           0, napp_ylos,       {0}         },
  { SDL_MOUSEBUTTONDOWN, 0, hiiri_alas,      {0}         },
  { SDL_WINDOWEVENT,     0, ikkunatapahtuma, {0}         },
};

void aja() {
 ALKU:
  while(SDL_PollEvent(&tapaht))
    for( int i=0; i<PITUUS(tapaht_sid); i++ )
      if( tapaht.type == tapaht_sid[i].tapahtuma )
	tapaht_sid[i].fun(tapaht_sid[i].arg);
  
  if(piirto_raidat) {
    piirra_raidat();
    piirto_raidat = 0;
  }
  SDL_RenderCopy( rend, tausta, NULL, NULL );
  piirra_kohdistin( kohdistin.x-kuvan_alku_x, kohdistin.r );
  if(toistaa) {
    if( (toiston_x = toiston_sijainti()) < 0 ||
	(valinta_x.a[0] >= 0 && toiston_x >= valinta_x.a[ valinta_x.a[1]>valinta_x.a[0] ]) )
      toistaa = 0;
    piirra_kohdistin(toiston_x, kohdistin.r);
  } else
    valinta_x.a[1] = kohdistin.x;
  piirra_valinta(&valinta_x);
  SDL_RenderPresent(rend);
  if(!jatka) {
    jatka=1;
    return;
  }
  SDL_Delay(15);
  goto ALKU;
}

void napp_alas(Arg turha) {
  for( int i=0; i<PITUUS(napp_alas_sid); i++ )
    if( tapaht.key.keysym.sym == napp_alas_sid[i].tapahtuma &&
	napp_alas_sid[i].mod == modkey_tuplana(modkey) )
      napp_alas_sid[i].fun(napp_alas_sid[i].arg);
  
  if( '0' <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9' )
    siirtoluku = 1<<(tapaht.key.keysym.sym-'0');
  switch(tapaht.key.keysym.sym) {
#define _MODKEYS_SWITCH_KEYDOWN
#include "modkeys.h"
  }
}

void napp_ylos(Arg turha) {
  switch(tapaht.key.keysym.sym) {
#define _MODKEYS_SWITCH_KEYUP
#include "modkeys.h"
  }
}

void hiiri_alas(Arg turha) {
  if( (hetki=hetkinyt()) - hiirihetki0 < tuplaklikkaus_ms ) {
    toista_kohdistin();
    return;
  }
  hiirihetki0 = hetki;
  kohdistin.x = tapaht.button.x + kuvan_alku_x;
  kohdistin.r = tapaht.button.y / raidan_kork;
}

void ikkunatapahtuma(Arg turha) {
  if( tapaht.window.event != SDL_WINDOWEVENT_RESIZED )
    return;
  SDL_GetWindowSize(ikkuna, &ikk_w, &ikk_h);
  SDL_DestroyTexture(tausta);
  tausta = SDL_CreateTexture( rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ikk_w, ikk_h );
  piirto_raidat = 1;
}

void kohdistin_sivulle(Arg arg) {
  kohdistin.x += arg.i*siirtoluku;
  if(kohdistin.x < 0)
    kohdistin.x = 0;
  else if(kohdistin.x > raidan_pit/ivali)
    kohdistin.x = raidan_pit/ivali;
}

void kohdistin_alas(Arg arg) {
  kohdistin.r = ( kohdistin.r + raitoja + arg.i ) % raitoja;
}

void kuvan_alku_sivulle(Arg arg) {
  kuvan_alku_x += siirtoluku*arg.i;
  if(kuvan_alku_x < 0) {
    kuvan_alku_x = 0;
    return;
  }
  int ylimeno_oikealla = kuvan_alku_x + ikk_w - raidan_pit/ivali;
  if(ylimeno_oikealla > 0)
    kuvan_alku_x -= ylimeno_oikealla;
  piirto_raidat = 1;
}

void zoomaa(Arg arg) {
  zoom *= arg.f;
  if( zoom < 1 )
    zoom = 1;
  /*päivitetään ivali,
    päivitetään kohdistin vastaamaan uutta ivalia,
    siirretään kuvan alkua, niin että kohdistin pysyy kuvassa paikallaan, jos mahdollista*/
  int vanha_kohd = kohdistin.x;
  unsigned vanha_siirtoluku = siirtoluku;
  kohdistin.x *= ivali;
  ivali = LASKE_IVALI;
  if( ivali <= 0 ) {
    ivali = 1;
    zoom = raidan_pit/(ikk_w*ivali);
  }
  kohdistin.x /= ivali;
  siirtoluku = kohdistin.x-vanha_kohd;
  kuvan_alku_sivulle((Arg){.i=1});
  siirtoluku = vanha_siirtoluku;
  piirto_raidat = 1;
}

void vaihda_toistaminen(Arg arg) {
  if(( toistaa = (toistaa+1)%2 )) {
    if( valinta_x.a[0] < 0 )
      toista_vali( (struct int2){{ kohdistin.x, raidan_pit/ivali }} );
    else
      toista_vali(valinta_x);
    return;
  }
  snd_pcm_drop(kahva);
  if(arg.v)
    kohdistin.x = *(int*)arg.v;
}

void alusta_valinta(Arg arg) {
  valinta_x.a[0] = valinta_x.a[1] = arg.v? *(int*)arg.v : -1;
}

void laita_unixaika(Arg arg) {
  uint64_t aika = aanen_loppuhetki - raidan_pit/48 + *(int*)arg.v*ivali/48;
  if(ulos_fno < 0) {
    printf("%lu\n", aika);
    return;
  }
  int apu = seuraavaksi_kohdan_unixaika;
  write(ulos_fno, &apu, 4);
  if((apu=write(ulos_fno, &aika, 8)) == 8)
    return;
  if(apu < 0) {
    perror("\033[31mVirhe kirjoittamisessa (laita_kohdan_unixaika)\033[0m");
    return;
  }
  fprintf(stderr, "\033[31mVirhe: kirjoitettiin vähemmän kuin pitäisi (laita_kohdan_unixaika)\033[0m\n");
}

void laita_aika(Arg arg) {
  int32_t aika_ms = *(int*)arg.v*ivali / 48;
  if(ulos_fno < 0) {
    printf("%i ms\n", aika_ms);
    return;
  }
  int montako;
  if((montako=write(ulos_fno, &aika_ms, 4)) == 4)
    return;
  if(montako < 0) {
    perror("\033[31mVirhe kirjoittamisessa (laita_kohdan_aika_ms)\033[0m");
    return;
  }
  fprintf(stderr, "\033[31mVirhe: kirjoitettiin vähemmän kuin pitäisi (laita_kohdan_aika_ms)\033[0m\n");
}

void int_nollaksi(Arg arg) {
  *(int*)arg.v = 0;
}

void piirra_raidat() {
  ivali = LASKE_IVALI;
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
    float* p = data+raita*raidan_pit + ivali*kuvan_alku_x;       //p viittaa sijaintiin ääniraidalla
    for(int x=0; x<ikk_w; x++)                                    //x on sijainti kuvassa
      for(int ii=0; ii<ivali; ii++,p++)                           //ii on näytöllä samaan x-kohtaan tulevan näytteen indeksi
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
  vntarect.x = vnta->a[pienempi] - kuvan_alku_x;
  vntarect.w = vnta->a[!pienempi] - vnta->a[pienempi];
  vntarect.h = ikk_h;
  ASETA_VARI(vntavari);
  SDL_RenderFillRect(rend, &vntarect);
}

int toiston_sijainti() {
  int r =  toiston_alku*ivali + (hetkinyt()-toistohetki0) * 48;
  if( r >= raidan_pit )
    r *= -1;
  return r/ivali;
}

void toista_kohdistin() {
  snd_pcm_drop(kahva);
  toistaa = 1;
  toistohetki0 = hetkinyt();
  toiston_alku = kohdistin.x;
  while(snd_pcm_writei( kahva, data+DATAxKOHTA(kohdistin.r, toiston_alku), raidan_pit-toiston_alku*ivali ) < 0) {
    snd_pcm_prepare(kahva); //fprintf(stderr, "Puskurin alitäyttö\n");
  }
}

void toista_vali(struct int2 vali) {
  snd_pcm_drop(kahva);
  toistaa = 1;
  toistohetki0 = hetkinyt();
  int pienempi = vali.a[1] < vali.a[0];
  toiston_alku = vali.a[pienempi];
  while(snd_pcm_writei( kahva, data+DATAxKOHTA( kohdistin.r, vali.a[pienempi] ), (vali.a[!pienempi]-vali.a[pienempi])*ivali ) < 0)
    snd_pcm_prepare(kahva);
}

uint64_t hetkinyt() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
}

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

static int oli_sdl = 0;
void aanen_valinta(float* data1, int raitoja1, int raidan_pit1, float* kynnysarvot1, snd_pcm_t* kahva1, int ulos_fno1) {
  raitoja = raitoja1; raidan_pit = raidan_pit1; kahva = kahva1; ulos_fno = ulos_fno1;
  data = malloc(raitoja*raidan_pit*sizeof(float));
  skaalat = malloc(raitoja*sizeof(float));
  memcpy(data, data1, raitoja*raidan_pit*sizeof(float));
  if(kynnysarvot1) {
    kynnysarvot = malloc(raitoja*sizeof(float));
    memcpy(kynnysarvot, kynnysarvot1, raitoja*sizeof(float));
  }
  if(SDL_WasInit(SDL_INIT_VIDEO))
    oli_sdl = 1;
  else if(SDL_Init(SDL_INIT_VIDEO)) {
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
