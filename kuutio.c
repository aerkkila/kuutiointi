#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <pthread.h> //käytetään vain laskentakuutiossa
#include "kuutio.h"
#include "muistin_jako.h"
#ifndef EI_SAVEL_MAKRO
#include "python_savel.h"
#endif

kuutio_t kuutio;
int3 akst[6];
int3 akst_tij[6];
SDL_Texture* alusta[2];
#ifndef __EI_AJANOTTOA__
void lue_siirrot(shmRak_s*);
int viimeViesti=0;
shmRak_s *ipc;
volatile float* savelPtr;
#endif

kuutio_t luo_kuutio(int N) {  
  kuutio.N = N;
  kuutio.N2 = N*N;
  kuutio.sivut = malloc(6*kuutio.N2);
  kuutio.ratkaistu = 1;
  for(int i=0; i<6; i++)
    memset(kuutio.sivut+i*kuutio.N2, i, kuutio.N2);
  return kuutio;
}

void siirto(kuutio_t* kuutp, int tahko, int siirtokaista, int maara) {
  if(maara == 0)
    return;
  int N = kuutp->N;
  if(siirtokaista < 0 || siirtokaista > N)
    return;
  else if(siirtokaista == N && N > 1) {
    maara = (maara+2) % 4;
    tahko = (tahko+3) % 6;
    siirtokaista = 1;
  }
  /*siirretään kuin old-pochman-menetelmässä:
    vaihdetaan ensin sivut 0,1, sitten 0,2 jne*/
  int iakseli = ABS(akst_tij[tahko].a[1]%3);
  int3 ruutu0 = hae_ruutu(tahko, 0, -siirtokaista);
  int IvaiJ = akst[ruutu0.a[0]].a[iakseli];
  for(int j=1; j<4; j++) {
    for(int i=0; i<N; i++) {
      ruutu0 = hae_ruutu(tahko, i, -siirtokaista); //alkukohdaksi valitaan j-akselin alimeno
      int etumerkki = SIGN(IvaiJ) * SIGN(akst[tahko].a[iakseli]);
      int3 ruutu1 = hae_ruutu(ruutu0.a[0],				\
			      ruutu0.a[1] + (2-ABS(IvaiJ)) * j*N * etumerkki, \
			      ruutu0.a[2] + (ABS(IvaiJ)-1) * j*N * etumerkki);
      VAIHDA(kuutp->sivut[SIVUINT3(ruutu0)], kuutp->sivut[SIVUINT3(ruutu1)], char);
    }
  }
  /*siivusiirrolle (slice move) kääntö on nyt suoritettu*/
  if(siirtokaista > 1) {
    siirto(kuutp, tahko, siirtokaista, maara-1);
    return;
  }
  /*käännetään käännetty sivu*/
  char* sivu = kuutp->sivut+tahko*N*N;
  char apu[N*N];
  memcpy(apu, sivu, N*N);

  /*nyt käännetään: */
#define arvo(taul,j,i) taul[(i)*N+(j)]
  for(int i=0; i<N; i++)
    for(int j=0; j<N; j++)
      *sivu++ = arvo(apu,N-1-i,j);
#undef arvo
  siirto(kuutp, tahko, siirtokaista, maara-1);
}

/*esim _u, 3, 0 (3x3x3-kuutio) --> _r, 2, 0:
  palauttaa oikean ruudun koordinaatit, kun yksi indeksi menee alunperin yli tahkolta*/
int3 hae_ruutu(int tahko0, int i0, int j0) {
  int IvaiJ = (i0<0)? -1: (i0>=kuutio.N)? 1: (j0<0)? -2: (j0>=kuutio.N)? 2: 0;
  if(!IvaiJ)
    return (int3){{tahko0, i0, j0}};
  
  int aksTahko0;
  int3 ruutu0 = (int3){{tahko0, i0, j0}};
  aksTahko0 = ABS(tahko0%3);
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

inline char __attribute__((always_inline)) onkoRatkaistu(kuutio_t* kuutp) {
  int N2 = kuutp->N*kuutp->N;
  for(int sivu=0; sivu<6; sivu++) {
    char* restrict s = kuutp->sivut+sivu*N2;
    for(int i=1; i<N2; i++)
      if(s[i] != *s)
	return 0;
  }
  return 1;
}

#ifndef __EI_GRAFIIKKAA__
#include "kuution_grafiikka.c"
#endif

int main(int argc, char** argv) {
  int N;
  if( argc < 2 || !(sscanf(argv[1], "%i", &N)) )
    N = 3;
  
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
  kuutio = luo_kuutio(N);
  
#ifndef __EI_SEKUNTIKELLOA__
  ipc = liity_muistiin();
  if(!ipc)
    return 1;
#endif

#ifdef __EI_GRAFIIKKAA__
#include "laskentakuutio.c"
#else //__on_grafiikka__
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
  free(kuva.ruudut);
  if(!oli_sdl)
    SDL_Quit();
#endif // __on_grafiikka__
  free(kuutio.sivut);
  return 0;
}
