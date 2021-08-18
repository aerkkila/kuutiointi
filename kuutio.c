#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdarg.h>
#include "kuutio.h"
#include "kuution_grafiikka.h"
#include "muistin_jako.h"
#include "kuution_kommunikointi.h"
#include "python_savel.h"

#define PI 3.14159265358979

kuutio_t kuutio;
kuva_t kuva;
int3 akst[6];
int3 akst_tij[6];
SDL_Texture* alusta[2];
#ifdef __KUUTION_KOMMUNIKOINTI__
int viimeViesti;
shmRak_s *ipc;
volatile float* savelPtr;
#endif

kuutio_t luo_kuutio(int N) {
  vari varit[6];
  varit[_u] = VARI(255,255,255); //valkoinen
  varit[_f] = VARI(0,  220,  0); //vihreä
  varit[_r] = VARI(255,0  ,0  ); //punainen
  varit[_d] = VARI(255,255,0  ); //keltainen
  varit[_b] = VARI(0,  0,  255); //sininen
  varit[_l] = VARI(220,120,0  ); //oranssi
  
  kuutio.N = N;
  int N2 = N*N;
  koordf asento = {{PI/6, -PI/6, 0}};
  kuutio.kannat[0] = puorauta((koordf){{1,0,0}}, asento);
  kuutio.kannat[1] = puorauta((koordf){{0,1,0}}, asento);
  kuutio.kannat[2] = puorauta((koordf){{0,0,1}}, asento);
  kuutio.ruudut = malloc(6*4*N2*sizeof(koordf));
  kuutio.sivut = malloc(6*N2);
  kuutio.varit = malloc(sizeof(varit));
  kuutio.ratkaistu = 1;
  
  for(int i=0; i<6; i++) {
    memset(kuutio.sivut+i*N2, i, N2);
    kuutio.varit[i] = varit[i];
  }
  tee_ruutujen_koordtit();
  return kuutio;
}

void tuhoa_kuutio() {
  free(kuutio.sivut);
  free(kuutio.ruudut);
  free(kuutio.varit);
  kuutio.ruudut = NULL;
  kuutio.sivut = NULL;
  kuutio.varit = NULL;
}

void paivita() {
  kuva.paivita = 0;
  SDL_SetRenderDrawColor(kuva.rend, 0, 0, 0, 255);
  SDL_RenderClear(kuva.rend);
  piirra_kuvaksi();
  if(kuva.korostus >= 0)
    korosta_tahko(kuva.korostus);
  if(kuva.ruutuKorostus.a[0] >= 0)
    korosta_ruutu(kuutio.ruudut+RUUTUINT3(kuva.ruutuKorostus), 3);
  SDL_RenderPresent(kuva.rend);
}

/*esim _u, 3, 0 (3x3x3-kuutio) --> _r, 2, 0:
  palauttaa oikean ruudun koordinaatit, kun yksi indeksi menee alunperin yli tahkolta*/
int3 hae_ruutu(int tahko0, int i0, int j0) {
  int IvaiJ = (i0<0)? -1: (i0>=kuutio.N)? 1: (j0<0)? -2: (j0>=kuutio.N)? 2: 0;
  if(!IvaiJ)
    return (int3){{tahko0, i0, j0}};
  
  int aksTahko0;
  int3 ruutu0 = (int3){{tahko0, i0, j0}};
  for(aksTahko0=0; aksTahko0<3; aksTahko0++)
    if(ABS(akst[tahko0].a[aksTahko0]) == 3)
      break;
  if((i0 < 0 || i0 >= kuutio.N) && (j0 < 0 || j0 >= kuutio.N))
    return (int3){{-1, -1, -1}};
  int ylimenoaks;
  for(ylimenoaks=0; ylimenoaks<3; ylimenoaks++)
    if(ABS(akst[tahko0].a[ylimenoaks]) == ABS(IvaiJ)) //akseli, jolta kys tahko menee yli
      break;
  int tahko1;
  int haluttu = 3*SIGN(IvaiJ)*SIGN(akst[tahko0].a[ylimenoaks]); //menosuunta ja lukusuunta
  for(tahko1=0; tahko1<6; tahko1++)
    if(akst[tahko1].a[ylimenoaks] == haluttu)
      break;
  int ij1[2], ind;
  /*tullaanko alkuun vai loppuun eli kasvaako uusi indeksi vanhan tahkon sijainnin suuntaan
    myös, että tullaanko i- vai j-akselin suunnasta*/
  int tulo = akst[tahko1].a[aksTahko0] * SIGN(akst[tahko0].a[aksTahko0]);
  ind = ABS(tulo)-1; //±1->0->i, ±2->1->j
#define A ruutu0.a[ABS(IvaiJ)]
  int lisa = (A<0)? -A-1: A-kuutio.N;
  ij1[ind] = (tulo<0)? 0 + lisa : kuutio.N-1 - lisa ; //negat --> tullaan negatiiviselta suunnalta
#undef A
  ind = (ind+1)%2;
  /*toisen indeksin akseli on molempiin tahkoakseleihin kohtisuorassa*/
  int akseli2 = 3-aksTahko0-ABS(ylimenoaks);
  ij1[ind] = (ABS(akst[tahko0].a[akseli2]) == 1)? i0: j0;
  /*vaihdetaan, jos ovat vastakkaissuuntaiset*/
  if(akst[tahko0].a[akseli2] * akst[tahko1].a[akseli2] < 0)
    ij1[ind] = kuutio.N-1 - ij1[ind];
  return hae_ruutu(tahko1, ij1[0], ij1[1]);
}

inline float __attribute__((always_inline)) vektpituus(koordf a) {
  return sqrt(pow(a.a[0],2) + pow(a.a[1],2));
}

inline float __attribute__((always_inline)) pistetulo(koordf a, koordf b) {
  return a.a[0]*b.a[0] + a.a[1]*b.a[1];
}

/*alueella, jos kierrosluku ≠ 0 eli ääriviivat kiertävät pisteen*/
int piste_alueella(float x, float y, int n, ...) {
  va_list argl;
  va_start(argl, n);
  float kl = 0;
  koordf sv1, sv2, a1, a2;
  koordf p = (koordf){{x, y, 0}};
  a1 = va_arg(argl, koordf);
  for(int i=1; i<=n; i++) {
    if(i==n)
      va_start(argl,n); //ympäri asti eli viimeisenä ensimmäinen
    a2 = va_arg(argl, koordf);
    sv1 = suuntavektori(&p, &a1);
    sv2 = suuntavektori(&p, &a2);
    kl += (acosf( pistetulo(sv1,sv2) / (vektpituus(sv1)*vektpituus(sv2)) ) * \
	   SIGN(ristitulo_z(sv1,sv2)));
    a1 = a2;
  }
  return round(kl);
}

int mika_tahko(int x, int y) {
  int tahko;
  for(tahko=0; tahko<6; tahko++) {
    /*näkyykö tahko*/
    if(ristitulo_z(suuntavektori(kuutio.ruudut+RUUTU(tahko,0,0),	\
				 kuutio.ruudut+RUUTU(tahko,0,kuutio.N-1)), \
		   suuntavektori(kuutio.ruudut+RUUTU(tahko,0,0),	\
				 kuutio.ruudut+RUUTU(tahko,kuutio.N-1,0))) <= 0)
      continue;
    /*näkyy, mutta onko oikea*/
    if(piste_alueella((float)x, (float)(-y), 4,				\
		      kuutio.ruudut[RUUTU(tahko,0,0)],			\
		      kuutio.ruudut[RUUTU(tahko,kuutio.N-1,0)+1],	\
		      kuutio.ruudut[RUUTU(tahko,kuutio.N-1,kuutio.N-1)+2], \
		      kuutio.ruudut[RUUTU(tahko,0,kuutio.N-1)+3]))
      return tahko;
  }
  return -1;
}

int3 mika_ruutu(int x, int y) {
  int tahko = mika_tahko(x,y);
  if(tahko < 0)
    goto EI_LOUTUNUT;
  float xf = x, yf = -y;
  float rMaks = 1.41421*kuva.resKuut/kuutio.N; //ruudun nurkan osuessa kauemmas hylätään heti
  for(int i=0; i<kuutio.N; i++)
    for(int j=0; j<kuutio.N; j++) {
      koordf* tr = kuutio.ruudut+RUUTU(tahko,i,j);
      if(fabs(tr->a[0]-xf) > rMaks)
	continue;
      if(fabs(tr->a[1]-yf) > rMaks)
	continue;
      if(piste_alueella(xf, yf, 4, tr[0], tr[1], tr[2], tr[3]))
	return (int3){{tahko,i,j}};
    }
 EI_LOUTUNUT:
  return (int3){{-1,-1,-1}};
}

void siirto(int tahko, int siirtokaista, int maara) {
  if(maara == 0)
    return;
  int N = kuutio.N;
  if(siirtokaista < 0 || siirtokaista > N)
    return;
  else if(siirtokaista == N) {
    maara = (maara+2) % 4;
    tahko = (tahko+3) % 6;
    siirtokaista = 1;
  }
  /*siirretään kuin old-pochman-menetelmässä:
    vaihdetaan ensin sivut 0,1, sitten 0,2 jne*/
  int iakseli;
  for(iakseli=0; iakseli<3; iakseli++)
    if(ABS(akst[tahko].a[iakseli]) == 1)
      break;
  int3 ruutu0 = hae_ruutu(tahko, 0, -siirtokaista);
  int IvaiJ = akst[ruutu0.a[0]].a[iakseli];
  for(int j=1; j<4; j++) {
    for(int i=0; i<N; i++) {
      ruutu0 = hae_ruutu(tahko, i, -siirtokaista); //alkukohdaksi valitaan j-akselin alimeno
      int etumerkki = SIGN(IvaiJ) * SIGN(akst[tahko].a[iakseli]);
      int3 ruutu1 = hae_ruutu(ruutu0.a[0],				\
			      ruutu0.a[1] + (2-ABS(IvaiJ)) * j*N * etumerkki, \
			      ruutu0.a[2] + (ABS(IvaiJ)-1) * j*N * etumerkki);
      VAIHDA(kuutio.sivut[SIVUINT3(ruutu0)], kuutio.sivut[SIVUINT3(ruutu1)], char);
    }
  }
  /*siivusiirrolle (slice move) kääntö on nyt suoritettu*/
  if(siirtokaista > 1 && siirtokaista < N) {
    siirto(tahko, siirtokaista, maara-1);
    return;
  }
  
  /*käännetään käännetty sivu*/
  int sivuInd;
  switch(tahko) {
  case _r:
    sivuInd = _r;
    if(siirtokaista == N)
      sivuInd = _l;
    break;
  case _l:
    sivuInd = _l;
    if(siirtokaista == N)
      sivuInd = _r;
    break;
  case _u:
    sivuInd = _u;
    if(siirtokaista == N)
      sivuInd = _d;
    break;
  case _d:
    sivuInd = _d;
    if(siirtokaista == N)
      sivuInd = _u;
    break;
  case _b:
    sivuInd = _b;
    if(siirtokaista == N)
      sivuInd = _f;
    break;
  case _f:
    sivuInd = _f;
    if(siirtokaista == N)
      sivuInd = _b;
    break;
  default:
    return;
  }
  char* sivu = kuutio.sivut+sivuInd*N*N;
  char apu[N*N];
  memcpy(apu, sivu, N*N);

  /*nyt käännetään: */
#define arvo(taul,j,i) taul[(i)*N+(j)]
  for(int i=0; i<N; i++)
    for(int j=0; j<N; j++)
      *sivu++ = arvo(apu,N-1-i,j);
#undef arvo
  siirto(tahko, siirtokaista, maara-1);
}

inline char __attribute__((always_inline)) onkoRatkaistu() {
  int N2 = kuutio.N*kuutio.N;
  for(int sivu=0; sivu<6; sivu++) {
    char* restrict s = kuutio.sivut+sivu*N2;
    for(int i=1; i<N2; i++)
      if(s[i] != *s)
	return 0;
  }
  return 1;
}

float kaantoaika;
float kaantoaika0 = 0.2;

static inline void __attribute__((always_inline)) animoi(int tahko, int kaista, int maara) {
  static double viimeKaantohetki = -1.0;
  struct timeval hetki;
  gettimeofday(&hetki, NULL);
  double hetkiNyt = hetki.tv_sec + hetki.tv_usec*1e-6;
  double erotus = hetkiNyt - viimeKaantohetki;
  if(erotus < kaantoaika0)
    kaantoaika = erotus;
  else
    kaantoaika = kaantoaika0;
  koordf akseli = kuutio.kannat[tahko%3];
  if(tahko/3)
    for(int i=0; i<3; i++)
      akseli.a[i] *= -1;
  kaantoanimaatio(tahko, kaista, akseli, maara-2, kaantoaika);
  tee_ruutujen_koordtit();
  kuva.paivita = 1;
  gettimeofday(&hetki, NULL);
  viimeKaantohetki = hetki.tv_sec + hetki.tv_usec*1e-6;
}

static inline void __attribute__((always_inline)) kaantoInl(char akseli, int maara) {
  if(!maara)
    return;
  if(maara < 0)
    maara += 4;
  int tahko;
  switch(akseli) {
  case 'x':
    tahko = _r;
    break;
  case 'y':
    tahko = _u;
    break;
  case 'z':
    tahko = _f;
    break;
  default:
    return;
  }
  animoi(tahko, -kuutio.N, maara);
  for(int m=0; m<maara; m++)
    for(int i=1; i<=kuutio.N; i++)
      siirto(tahko, i, 1);
  kuva.paivita = 1;
}

static inline void __attribute__((always_inline)) siirtoInl(int tahko, int kaista, int maara) {
  animoi(tahko, kaista, maara);
  siirto(tahko, kaista, maara);
  kuva.paivita = 1;
  kuutio.ratkaistu = onkoRatkaistu();
#ifdef __KUUTION_KOMMUNIKOINTI__
  if(viimeViesti == ipcTarkastelu) {
    ipc->viesti = ipcAloita;
    viimeViesti = ipcAloita;
  } else if(viimeViesti == ipcAloita && kuutio.ratkaistu) {
    ipc->viesti = ipcLopeta;
    viimeViesti = ipcLopeta;
  }
#endif
}

int main(int argc, char** argv) {
  char ohjelman_nimi[] = "Kuutio";
  int ikkuna_x = 300;
  int ikkuna_y = 300;
  int ikkuna_w = 500;
  int ikkuna_h = 500;
  int N;
  char oli_sdl = 0;
  if(argc < 2)
    N = 3;
  else
    sscanf(argv[1], "%i", &N);
  
  if(!SDL_WasInit(SDL_INIT_VIDEO))
    SDL_Init(SDL_INIT_VIDEO);
  else
    oli_sdl = 1;

  /*akselit, näissä 0 korvataan 3:lla jotta saadaan etumerkki*/
  /*esim. oikealla (r) j liikuttaa negatiiviseen y-suuntaan (1.indeksi = y, ±2 = j)
  i liikuttaa negatiiviseen z-suuntaan (2. indeksi = z, ±1 = i)
  sijainti on positiivisella x-akselilla (0. indeksi = x, ±3 = sijainti)*/
  akst[_r] = (int3){{3, -2, -1}};
  akst[_l] = (int3){{-3, -2, 1}};
  akst[_u] = (int3){{1, 3, 2}};
  akst[_d] = (int3){{1, -3, -2}};
  akst[_f] = (int3){{1, -2, 3}};
  akst[_b] = (int3){{-1, -2, -3}};

  /*käänteinen yllä olevaan: luvut ovat tämä, i, j
    oikea on +x-akselilla (0. = tämä), i on -z-akseli (1. = i, ±2 = z) jne*/
  akst_tij[_r] = (int3){{3, -2, -1}};
  akst_tij[_l] = (int3){{-3, 2, -1}};
  akst_tij[_u] = (int3){{1, 3, 2}};
  akst_tij[_d] = (int3){{-1, 3, -2}};
  akst_tij[_f] = (int3){{2, 3, -1}};
  akst_tij[_b] = (int3){{-2, -3, -1}};

  /*Kuvan tekeminen*/
  kuva.ikkuna = SDL_CreateWindow\
    (ohjelman_nimi, ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h, SDL_WINDOW_RESIZABLE);
  kuva.rend = SDL_CreateRenderer(kuva.ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);
  if(!kuva.rend)
    goto ULOS;
  kuva.xRes = ikkuna_w;
  kuva.yRes = ikkuna_h;
  kuva.mustaOsuus = 0.05;
  kuva.paivita = 1;
  kuva.korostus = -1;
  kuva.ruutuKorostus = (int3){{-1, kuutio.N/2, kuutio.N/2}};
  kuva.korostusVari = VARI(80, 233, 166);
  kuva.resKuut = (ikkuna_h < ikkuna_w)? ikkuna_h/sqrt(3.0)/2 : ikkuna_w/sqrt(3.0);
  kuva.sij0 = (ikkuna_h < ikkuna_w)? (ikkuna_h-kuva.resKuut)/2: (ikkuna_w-kuva.resKuut)/2;

  kuutio = luo_kuutio(N);
  kaantoaika = kaantoaika0;
  for(int i=0; i<2; i++) {
    alusta[i] = SDL_CreateTexture(kuva.rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, kuva.xRes, kuva.yRes);
    SDL_SetTextureBlendMode(alusta[i], SDL_BLENDMODE_BLEND); //alustan kopioinnissa on alfa-kanava
    if(!alusta[i])
      return 1;
  }
  SDL_SetRenderDrawBlendMode(kuva.rend, SDL_BLENDMODE_NONE); //muualla otetaan sellaisenaan
  
#ifdef __KUUTION_KOMMUNIKOINTI__
  ipc = liity_muistiin();
  if(!ipc)
    return 1;
#endif

#define siirtoInl1(tahko, maara) siirtoInl(tahko, siirtokaista, maara)
 
  SDL_Event tapaht; 
  int xVanha, yVanha;
  char hiiri_painettu = 0;
  int siirtokaista = 1;
  int raahattiin = 0;
  int vaihto = 0;
  int numero1_pohjassa = 0;
  int numero10_pohjassa = 0;
  while(1) {
    while(SDL_PollEvent(&tapaht)) {
      switch(tapaht.type) {
      case SDL_QUIT:
	goto ULOS;
      case SDL_WINDOWEVENT:
	switch(tapaht.window.event) {
	case SDL_WINDOWEVENT_RESIZED:;
	  int koko1 = (tapaht.window.data1 < tapaht.window.data2)? tapaht.window.data1: tapaht.window.data2;
	  kuva.xRes = tapaht.window.data1;
	  kuva.yRes = tapaht.window.data2;
	  for(int i=0; i<2; i++) {
	    SDL_DestroyTexture(alusta[i]);
	    if(!(alusta[i] = SDL_CreateTexture(kuva.rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, kuva.xRes, kuva.yRes)))
	      printf("Ei tehty alustaa %i\n", i);
	    SDL_SetTextureBlendMode(alusta[i], SDL_BLENDMODE_BLEND);
	  }
	  kuva.resKuut = koko1/sqrt(3.0);
	  
	  if((koko1-kuva.resKuut)/2 != kuva.sij0) {
	    kuva.sij0 = (koko1-kuva.resKuut)/2;
	    tee_ruutujen_koordtit();
	    kuva.paivita = 1;
	  }
	  break;
	}
	break;
      case SDL_KEYDOWN:
	switch(tapaht.key.keysym.scancode) {
	case SDL_SCANCODE_I:
	  siirtoInl1(_u, 1);
	  break;
	case SDL_SCANCODE_L:
	  siirtoInl1(_r, 1);
	  break;
	case SDL_SCANCODE_J:
	  siirtoInl1(_r, 3);
	  break;
	case SDL_SCANCODE_PERIOD:
	  siirtoInl1(_d, 3);
	  break;
	case SDL_SCANCODE_K:
	  siirtoInl1(_f, 1);
	  break;
	case SDL_SCANCODE_O:
	  siirtoInl1(_b, 3);
	  break;
	case SDL_SCANCODE_E:
	  siirtoInl1(_u, 3);
	  break;
	case SDL_SCANCODE_F:
	  siirtoInl1(_l, 1);
	  break;
	case SDL_SCANCODE_S:
	  siirtoInl1(_l, 3);
	  break;
	case SDL_SCANCODE_X:
	  siirtoInl1(_d, 1);
	  break;
	case SDL_SCANCODE_D:
	  siirtoInl1(_f, 3);
	  break;
	case SDL_SCANCODE_W:
	  siirtoInl1(_b, 1);
	  break;
	case SDL_SCANCODE_U:
	  for(int kaista=2; kaista<N; kaista++)
	    siirtoInl(_r, kaista, 1);
	  break;
	case SDL_SCANCODE_R:
	  for(int kaista=2; kaista<N; kaista++)
	    siirtoInl(_l, kaista, 1);
	  break;
	  /*käytetään kääntämisten määrässä siirtokaistaa*/
	case SDL_SCANCODE_H:
	  kaantoInl('y', (3*(siirtokaista)) % 4);
	  break;
	case SDL_SCANCODE_G:
	  kaantoInl('y', (1*(siirtokaista)) % 4);
	  break;
	case SDL_SCANCODE_N:
	  kaantoInl('x', (1*(siirtokaista)) % 4);
	  break;
	case SDL_SCANCODE_V:
	  kaantoInl('x', (3*(siirtokaista)) % 4);
	  break;
	case SDL_SCANCODE_COMMA:
	  kaantoInl('z', (1*(siirtokaista)) % 4);
	  break;
	case SDL_SCANCODE_C:
	  kaantoInl('z', (3*(siirtokaista)) % 4);
	  break;
	default:
	  switch(tapaht.key.keysym.sym) {
	  case SDLK_RSHIFT:
	  case SDLK_LSHIFT:
	    siirtokaista++;
	    vaihto = 1;
	    break;
	  case SDLK_PAUSE:
	    if(vaihto)
	      asm("int $3");
	    break;
	  case SDLK_RETURN:
	    kaantoanimaatio(_f, 0, (koordf){{0,1,0}}, 1.0, 1);
	    break;
#define A kuva.ruutuKorostus
#define B(i) kuva.ruutuKorostus.a[i]
	  case SDLK_LEFT:
	    A = hae_ruutu(B(0), B(1)-1, B(2));
	    kuva.paivita=1;
	    break;
	  case SDLK_RIGHT:
	    A = hae_ruutu(B(0), B(1)+1, B(2));
	    kuva.paivita=1;
	    break;
	  case SDLK_UP:
	    if(A.a[0] < 0)
	      A = (int3){{_f, kuutio.N/2, kuutio.N/2}};
	    else if(vaihto)
	      A.a[0] = -1;
	    else
	      A = hae_ruutu(B(0), B(1), B(2)-1);
	    kuva.paivita=1;
	    break;
	  case SDLK_DOWN:
	    A = hae_ruutu(B(0), B(1), B(2)+1);
	    kuva.paivita=1;
	    break;
#undef A
#undef B
#ifdef __KUUTION_KOMMUNIKOINTI__
	  case SDLK_F1:
	    lue_siirrot(ipc);
	    ipc->viesti = ipcTarkastelu;
	    viimeViesti = ipcTarkastelu;
	    break;
	  case SDLK_SPACE:
	    if(viimeViesti == ipcAloita) {
	      ipc->viesti = ipcLopeta;
	      viimeViesti = ipcLopeta;
	    } else {
	      ipc->viesti = ipcAloita;
	      viimeViesti = ipcAloita;
	    }
	    break;
#endif
#ifdef __PYTHON_SAVEL__
	  case SDLK_F2:; //käynnistää tai sammuttaa sävelkuuntelijan
	    if(!savelPtr) {
	      pid_t pid1, pid2;
	      if((pid1 = fork()) < 0) {
		perror("Haarukkavirhe pid1");
		break;
	      } else if(!pid1) {
		if((pid2 = fork()) < 0) {
		  perror("Haarukkavirhe pid2");
		  exit(1);
		} else if (pid2) {
		  _exit(0);
		} else {
		  if(system("./sävel.py") < 0)
		    perror("sävel.py");
		  exit(0);
		}
	      } else
		waitpid(pid1, NULL, 0);
	      savelPtr = savelmuistiin();
	      *savelPtr = -1.0;
	      break;
	    } else {
	      savelPtr = sulje_savelmuisti((void*)savelPtr);
	    }
#endif
	  default:
	    if('1' <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9' && !numero1_pohjassa) {
	      numero1_pohjassa = 1;
	      siirtokaista += tapaht.key.keysym.sym - '1';
	    } else if(SDLK_KP_1 <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= SDLK_KP_9 && !numero10_pohjassa) {
	      numero10_pohjassa = 1;
	      siirtokaista += (tapaht.key.keysym.sym - SDLK_KP_1 + 1) * 10;
	    }
	    break;
	  }
	  break;
	}
	break;
      case SDL_KEYUP:
	switch(tapaht.key.keysym.sym) {
	case SDLK_RSHIFT:
	case SDLK_LSHIFT:
	  vaihto = 0;
	  siirtokaista = 1;
	  break;
	default:
	  if('1' <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9') {
	    numero1_pohjassa = 0;
	    if(!vaihto)
	      siirtokaista = 1;
	  } else if(SDLK_KP_1 <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= SDLK_KP_9) {
	    numero10_pohjassa = 0;
	    if(!vaihto)
	      siirtokaista = 1;
	  }
	  break;
	}
	break;
      case SDL_MOUSEBUTTONDOWN:
	xVanha = tapaht.button.x;
	yVanha = tapaht.button.y;
	hiiri_painettu = 1;
	raahattiin = 0;
	if(tapaht.button.button == SDL_BUTTON_RIGHT)
	  hiiri_painettu = 2;
	break;
      case SDL_MOUSEMOTION:;
	/*pyöritetään, raahauksesta hiirellä*/
	if(hiiri_painettu) {
	  raahattiin = 1;
	  float xEro = (tapaht.motion.x-xVanha) * PI/(2*kuva.resKuut); //vasemmalle negatiivinen
	  float yEro = (tapaht.motion.y-yVanha) * PI/(2*kuva.resKuut); //alas positiivinen
	  xVanha = tapaht.motion.x;
	  yVanha = tapaht.motion.y;
	  koordf akseli = {{[1] = xEro}};
	  akseli.a[hiiri_painettu & 2] = yEro; // x- tai z-akseli
	  for(int i=0; i<3; i++)
	    kuutio.kannat[i] = puorauta(kuutio.kannat[i], akseli);
	  
	  tee_ruutujen_koordtit();
	  kuva.paivita = 1;
	  break;
	}
	break;
      case SDL_MOUSEBUTTONUP:
	hiiri_painettu = 0;
	if(!raahattiin) {
	  int tahko = mika_tahko(tapaht.motion.x, tapaht.motion.y);
	  if(tahko >= 0)
	    siirtoInl(tahko, siirtokaista, (tapaht.button.button == 1)? 3: 1);
	}
	break;
      }
    } //while poll_event
#ifdef __PYTHON_SAVEL__
    if(savelPtr) {
      static char suunta = 1;
      static double savLoppuHetki = 1.0;
      float savel = *savelPtr;
      if(savel < 0) {
	if(savLoppuHetki < 0) { //sävel päättyi juuri
	  struct timeval hetki;
	  gettimeofday(&hetki, NULL);
	  savLoppuHetki = hetki.tv_sec + hetki.tv_usec*1.0/1000000;
	} else if(savLoppuHetki > 2) { // 1 olisi merkkinä, ettei tehdä mitään
	  struct timeval hetki;
	  gettimeofday(&hetki, NULL);
	  double hetkiNyt = hetki.tv_sec + hetki.tv_usec*1.0/1000000;
	  if(hetkiNyt - savLoppuHetki > 1.0 && kuva.korostus >= 0) {
	    siirtoInl1(kuva.korostus, suunta);
	    kuva.korostus = -1;
	    savLoppuHetki = 1;
	    *savelPtr = -1.0;
	  }
	}
	goto EI_SAVELTA;
      }
      savLoppuHetki = -1.0;
      int puoliask = savel_ero(savel);
      printf("%i\n", puoliask);
      *savelPtr = -1.0;
      suunta = 1;
      if(puoliask < 0) {
	puoliask += 12;
	suunta = 3;
      }
      switch(puoliask) {
      case 0:
	kuva.korostus = _r;
	break;
      case 2:
	kuva.korostus = _l;
	break;
      case 4:
	kuva.korostus = _u;
	break;
      case 5:
	kuva.korostus = _d;
	break;
      case 7:
	kuva.korostus = _f;
	break;
      case 10:
	kuva.korostus = _b;
	break;
      default:
	goto EI_SAVELTA;
      }
      kuva.paivita = 1;
    }
  EI_SAVELTA:
#endif
    if(kuva.paivita)
      paivita();
    SDL_Delay(20);
  }
  
 ULOS:
#ifdef __PYTHON_SAVEL__
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
  tuhoa_kuutio(kuutio);
  if(!oli_sdl)
    SDL_Quit();
  return 0;
}
