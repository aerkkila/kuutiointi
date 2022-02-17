#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <math.h>
#include <sys/wait.h>
#include "äänireuna.h"

static void putki0_tapahtumat();
static void opettaminen();

snd_pcm_t* kahva_capt;
snd_pcm_t* kahva_play;
const int taaj = 48000;
const int jaksonaika_ms = 30;
int gauss_sigma_kpl = 70;
int maksimin_alue = 30; //yhteen suuntaan
float kynnysarvot[] = {NAN, NAN, NAN, 1e-4};
long unsigned alku_kpl, suod_kpl, deri_kpl, ohen_kpl, pit_data, pit_jakso;
unsigned tarkka_taaj;
snd_pcm_uframes_t buffer_size;
int virhe_id;
float* data[6];
float* kokodata;
enum {raaka=2, suodate, derivoitu, ohennus};
#define ALSAFUNK(fun,...) if( (virhe_id = fun(__VA_ARGS__)) < 0 )	\
    {									\
      fprintf(stderr,"\033[31mVirhe funktiossa %s (äänireuna.c):\033[0m %s\n",#fun,snd_strerror(virhe_id)); \
      return virhe_id;							\
    }
static uint32_t kuluma_ms = 0;
static struct pollfd poll_0[] = { {-1, POLLIN}, {STDIN_FILENO, POLLIN} };

int tallennusaika_ms = 5000;
uint32_t opetusviive_ms = -1;
int p00=-1, p11=-1, apuint;

void sulje_putki(void** putki) {
  close( *( (int*)putki ) );
}

/*komentoriviargumentit*/
const struct {char* nimi; char* muoto; void* muuttujat[3]; void (*funktio)(void**); void** funargt;} kntoarg[] = {
  { "--tallennusaika_ms", "%i", {&tallennusaika_ms} },
  { "--opetusviive_ms", "%i", {&opetusviive_ms} },
  { "--gauss_sigma_kpl", "%i", {&gauss_sigma_kpl} },
  { "--kynnysarvo", "%e", {kynnysarvot+3} },
  { "--putki0", "%i", {&p00, &apuint}, sulje_putki, (void**)&apuint },
  { "--putki1", "%i", {&apuint, &p11}, sulje_putki, (void**)&apuint },
};

int alusta_aani(snd_pcm_t** kahva, snd_pcm_stream_t suoratoisto) {
  snd_pcm_hw_params_t* hwparams;
  ALSAFUNK(snd_pcm_open, kahva, "default", suoratoisto, 0);                                     //avaa
  snd_pcm_hw_params_alloca(&hwparams);
  ALSAFUNK(snd_pcm_hw_params_any, *kahva, hwparams);                                            //params_any
  ALSAFUNK(snd_pcm_hw_params_set_access, *kahva, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);      //access
  ALSAFUNK(snd_pcm_hw_params_set_rate_resample, *kahva, hwparams, 0);
  ALSAFUNK(snd_pcm_hw_params_set_format, *kahva, hwparams, SND_PCM_FORMAT_FLOAT_LE);            //format
  tarkka_taaj = taaj;
  ALSAFUNK(snd_pcm_hw_params_set_rate_near, *kahva, hwparams, &tarkka_taaj, 0);                 //rate
  if(taaj != tarkka_taaj)
    printf("Käytetään näytteenottotaajuutta %i taajuuden %i sijaan\n", tarkka_taaj, taaj);
  ALSAFUNK(snd_pcm_hw_params_set_channels, *kahva, hwparams, 1);                                //channels
  ALSAFUNK(snd_pcm_hw_params,*kahva,hwparams);
  snd_pcm_hw_params_get_buffer_size(hwparams,&buffer_size);
  snd_pcm_prepare(*kahva);
  return 0;
}

void siirra_dataa(float* data, int pit_data, int siirto) {
  for(int i=0; i+siirto<pit_data; i++)
    data[i] = data[i+siirto];
}

#define KERROIN 0.39894228 // 1/sqrt(2*pi)
#define GAUSSPAINO(t,sigma) ( KERROIN/sigma * exp(-0.5*(t)*(t)/(sigma*sigma)) )

void suodata(float* data, float* kohde, int pit_data, int pit_sigma) {
  int gausspit = pit_sigma*3;
  float gausskertoimet[gausspit*2+1];
  float* gausskert = gausskertoimet + gausspit; //osoitin keskikohtaan
  for(int t=0; t<=gausspit; t++)
    gausskert[t] = gausskert[-t] = GAUSSPAINO(t,pit_sigma);
  for(int i=gausspit; i+gausspit<pit_data; i++) {
    float summa = 0;
    for(int T=-gausspit; T<=gausspit; T++)
      summa += data[i+T]*gausskert[T];
    kohde[i] = summa;
  }
}

void derivaatta(float* data, float* kohde, int pit) {
  for(int i=0; i<pit-1; i++)
    kohde[i] = data[i+1]-data[i];
}

void ohentaminen(float* data, float* kohde, int pit, int maksimin_alue) {
  for(int i=maksimin_alue; i+maksimin_alue<pit; i++)
    for(int j=1; j<=maksimin_alue; j++) {
      if( ! (data[i-j] < data[i] && data[i] > data[i+j]) ) {
	kohde[i] = 0;
	break;
      }
      kohde[i] = data[i];
    }
}

void havaitse_ylitykset_kaikki(float** data) {
  suodata(data[raaka], data[suodate], pit_data, gauss_sigma_kpl);
  derivaatta(data[suodate], data[derivoitu], pit_data);
  memset(data[ohennus], 0, maksimin_alue*sizeof(float));
  memset(data[ohennus]+pit_data-maksimin_alue, 0, maksimin_alue*sizeof(float));
  ohentaminen(data[derivoitu], data[ohennus], pit_data, maksimin_alue);
}

void havaitse_ylitykset_jakso(float** data, int alkukohta) {
  suodata(data[raaka]+alkukohta, data[suodate], alku_kpl, gauss_sigma_kpl);
  derivaatta(data[suodate], data[derivoitu], suod_kpl);
  ohentaminen(data[derivoitu], data[ohennus], deri_kpl, maksimin_alue);
  for(int i=0; i<ohen_kpl; i++) {
    if( data[ohennus][i]>=kynnysarvot[3] ) {
      if(p11 < 0)
	printf("%.3e\n", data[ohennus][i]);
      else
	write(p11, data[ohennus]+i, sizeof(float));
    }
  }
}

int kumpi_valmis = -1;
int kumpaa_luetaan = -1;
int nauh_jatka = 1;

void sigint(int sig) {
  nauh_jatka = 0;
}
void sigchld(int sig) {
  while(waitpid(-1, NULL, WNOHANG)>0);
}

void* nauhoita(void* datav) {
  float** data = datav;
  int id = 0;
  while(nauh_jatka) {
    if(id == kumpaa_luetaan) {
      fprintf(stderr, "\033[31mVaroitus: kaikkea dataa ei ehditty käsitellä\033[0m\n");
      while(id == kumpaa_luetaan)
	usleep(1000);
    }
    while(snd_pcm_readi( kahva_capt, data[id], pit_jakso ) < 0) {
      snd_pcm_prepare(kahva_capt);
      fprintf(stderr, "Puskurin ylitäyttö\n");
    }
    kumpi_valmis = id;
    id = (id + 1) % 2;
  }
  return NULL;
}

void kasittele(void* datav) {
  float** data = datav;
  if(pit_data-alku_kpl < 0) {
    fprintf(stderr, "Liian lyhyt tallennusaika suhteessa muihin parametreihin\n");
    nauh_jatka = 0;
  }
  while(nauh_jatka) {
    while(kumpi_valmis < 0)
      usleep(1000);
    int id = kumpi_valmis;
    kumpaa_luetaan = id;
    siirra_dataa(data[raaka], pit_data, pit_jakso);
    memcpy(data[raaka]+pit_data-pit_jakso, data[id], pit_jakso*sizeof(float));
    havaitse_ylitykset_jakso(data, pit_data-alku_kpl);
    if((kuluma_ms+=jaksonaika_ms) >= opetusviive_ms) {
      opettaminen();
      kuluma_ms = 0;
    }
    putki0_tapahtumat();
    kumpaa_luetaan = -1; //merkki nauhoitusfunktiolle
    kumpi_valmis = -1; //merkki tälle funktiolle
  }
}

void aanen_valinta(float* kokodata, int raitoja, int raidan_pit, float* kynnysarvot, snd_pcm_t* kahva_play); //kirjastosta
void opettaminen() {
  havaitse_ylitykset_kaikki(data);
  aanen_valinta(kokodata+raaka*pit_jakso, ohennus-raaka+1, pit_data, kynnysarvot, kahva_play);
}

void putki0_tapahtumat() {
  int apu;
  if( !(apu=poll(poll_0, 2, 0)) )
    return;
  if(apu < 0) {
    fprintf(stderr, "Virhe (äänireuna->putki0_tapahtumat->poll): %s\n", strerror(errno));
    return;;
  }
  uint8_t viesti;
  for(int i=0; i<2; i++) {
    if(!poll_0[i].revents)
      continue;
    if(poll_0[i].revents & POLLIN) {
      if( (apu=read(poll_0[i].fd, &viesti, 1)) == 1 )
	break;
      else if(apu < 0)
	fprintf(stderr, "Virhe putken luennassa (äänireuna->putki0_tapahtumat): %s\n", strerror(errno));
      else
	fprintf(stderr, "Viestiä ei luettu, vaikka oli muka saatavilla (äänireuna->putki0_tapahtumat)\n");
      return;
    } else if(poll_0[i].revents & POLLHUP) {
      printf("Luettava putki %i sulkeutui. Ääniohjelma yrittää lopettaa.\n", poll_0[i].fd);
      nauh_jatka = 0;
      return;
    } else if(poll_0[i].revents & POLLERR)
      fprintf(stderr, "Virhetila putkessa %i (äänireuna->putki0_tapahtumat)\n", poll_0[i].fd);
    return;
  }
  if(viesti==aanireuna_opettaminen)
    opettaminen();
}

void lue_kntoriviargt(int argc, char** argv) {
  int pit = sizeof(kntoarg) / sizeof(kntoarg[0]);
  for(int i=1; i<argc; i++) {
    for(int j=0; j<pit; j++) {
      if( strcmp( argv[i], kntoarg[j].nimi ) )
	continue;
      for(int k=0; kntoarg[j].muuttujat[k]; k++)
	if( sscanf(argv[++i], kntoarg[j].muoto, kntoarg[j].muuttujat[k]) != 1 ) {
	  fprintf(stderr, "\033[31mVirhe\033[0m argumentin \"%s\" jälkeen\n", kntoarg[j].nimi);
	  exit(1);
	}
      if(kntoarg[j].funktio)
	kntoarg[j].funktio(kntoarg[j].funargt);
      goto SEURAAVA_ARGUMENTTI;
    }
    fprintf(stderr, "\033[31mTuntematon argumentti:\033[0m %s\n", argv[i]);
    exit(1);
  SEURAAVA_ARGUMENTTI:
  }
}

int main(int argc, char** argv) {
  lue_kntoriviargt(argc, argv);
  poll_0[0].fd = p00;
  signal(SIGINT, sigint);
  signal(SIGPIPE, sigint);
  signal(SIGCHLD, sigchld);
  pthread_t saie;
  pit_data = taaj*tallennusaika_ms/1000;
  pit_jakso = taaj*jaksonaika_ms/1000;
  alku_kpl = pit_jakso + gauss_sigma_kpl*6 + 1 + maksimin_alue*2;
  suod_kpl = alku_kpl - gauss_sigma_kpl*6;
  deri_kpl = suod_kpl - 1;
  ohen_kpl = suod_kpl - maksimin_alue*2; // == pit_jakso
  alusta_aani(&kahva_capt, SND_PCM_STREAM_CAPTURE);
  alusta_aani(&kahva_play, SND_PCM_STREAM_PLAYBACK);
  kokodata = calloc(pit_data*(ohennus-raaka+1) + pit_jakso*2, sizeof(float));
  for(int i=0; i<raaka; i++)
    data[i] = kokodata + i*pit_jakso;
  for(int i=raaka; i<=ohennus; i++)
    data[i] = kokodata + raaka*pit_jakso + (i-raaka)*pit_data;

  /*Asetetaan epäluvut dataan. Tarvittavan alueen päälle kirjoitetaan myöhemmin muuta.*/
  for(int i=pit_jakso*2; i<pit_data*(ohennus-raaka+1) + pit_jakso*2; i++)
    kokodata[i] = NAN;

  pthread_create(&saie, NULL, nauhoita, data);
  kasittele(data);
  pthread_join(saie, NULL);
  snd_pcm_close(kahva_capt);
  if(p00 >= 0) {
    close(p00);
    p00 = -1;
    poll_0[0].fd = p00;
  }
  if(p11 >= 0) {
    close(p11);
    p11 = -1;
  }
  free(kokodata);
  puts("\nÄäniohjelma lopetti");
  return 0;
}
