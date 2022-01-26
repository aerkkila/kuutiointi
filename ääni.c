#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

snd_pcm_t* kahva_capt;
snd_pcm_t* kahva_play;
int putki_ulos[2] = {-1,-1};
const int taaj = 48000;
const int tallennusaika_ms = 3000;
const int jaksonaika_ms = 30;
const int gauss_sigma_kpl = 70;
const int maksimin_alue = 30; //yhteen suuntaan
int alku_kpl, suod_kpl, deri_kpl, ohen_kpl, pit_data, pit_jakso;
unsigned tarkka_taaj;
snd_pcm_uframes_t buffer_size;
int virhe_id;
enum {raaka=2, suodate, derivoitu, ohennus};
#define ALSAFUNK(fun,...) if( (virhe_id = fun(__VA_ARGS__)) < 0 )	\
    {									\
      fprintf(stderr,"Virhe funktiossa %s:\n%s\n",#fun,snd_strerror(virhe_id)); \
      return virhe_id;							\
    }

int alusta_aani(snd_pcm_stream_t suoratoisto) {
  snd_pcm_hw_params_t* hwparams;
  ALSAFUNK(snd_pcm_open, &kahva_capt, "default", suoratoisto, 0);                                   //avaa
  snd_pcm_hw_params_alloca(&hwparams);
  ALSAFUNK(snd_pcm_hw_params_any, kahva_capt, hwparams);                                            //params_any
  ALSAFUNK(snd_pcm_hw_params_set_access, kahva_capt, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);      //access
  ALSAFUNK(snd_pcm_hw_params_set_rate_resample, kahva_capt, hwparams, 0);
  ALSAFUNK(snd_pcm_hw_params_set_format, kahva_capt, hwparams, SND_PCM_FORMAT_FLOAT_LE);            //format
  tarkka_taaj = taaj;
  ALSAFUNK(snd_pcm_hw_params_set_rate_near, kahva_capt, hwparams, &tarkka_taaj, 0);                 //rate
  if(taaj != tarkka_taaj)
    printf("Käytetään näytteenottotaajuutta %i taajuuden %i sijaan\n", tarkka_taaj, taaj);
  ALSAFUNK(snd_pcm_hw_params_set_channels, kahva_capt, hwparams, 1);                                //channels 
  ALSAFUNK(snd_pcm_hw_params,kahva_capt,hwparams);
  snd_pcm_hw_params_get_buffer_size(hwparams,&buffer_size);
  snd_pcm_prepare(kahva_capt);
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
    kohde[i-gausspit] = summa;
  }
}

void derivaatta(float* data, float* kohde, int pit) {
  for(int i=0; i<pit-1; i++)
    kohde[i] = data[i+1]-data[i];
}

void ohentaminen(float* data, float* kohde, int pit, int maksimin_alue) {
  for(int i=maksimin_alue,k=0; i+maksimin_alue<pit; i++,k++)
    for(int j=1; j<=maksimin_alue; j++) {
      if( ! (data[i-j] < data[i] && data[i] > data[i+j]) ) {
	kohde[k] = 0;
	break;
      }
      kohde[k] = data[i];
    }
}

void havaitse_reunat(float** data, int alkukohta) {
  suodata(data[raaka]+alkukohta, data[suodate], alku_kpl, gauss_sigma_kpl);
  derivaatta(data[suodate], data[derivoitu], suod_kpl);
  ohentaminen(data[derivoitu], data[ohennus], deri_kpl, maksimin_alue);
  for(int i=0; i<ohen_kpl; i++)
    if(data[ohennus][i] > 1.0e-5) {
      if(putki_ulos[1] < 0)
	printf("%.3e\n", data[ohennus][i]);
      else
	write(putki_ulos[1], data[ohennus]+i, sizeof(float));
    }
}

int kumpi_valmis = -1;
int kumpaa_luetaan = -1;
int nauh_jatka = 1;

void sigint_f(int sig) {
  nauh_jatka = 0;
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
    while(snd_pcm_readi( kahva_capt, data[id], taaj*jaksonaika_ms/1000 ) < 0) {
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
    havaitse_reunat(data, pit_data-alku_kpl);
    kumpaa_luetaan = -1; //merkki nauhoitusfunktiolle
    kumpi_valmis = -1; //merkki tälle funktiolle
  }
}

int main(int argc, char** argv) {
  for(int i=1; i<argc; i++) {
    if(!strcmp(argv[i], "--putki_ulos")) {
      if(argc <= i+2 || sscanf(argv[i+1], "%i", putki_ulos)!=1 || sscanf(argv[i+2], "%i", putki_ulos+1)!=1) {
	fprintf(stderr, "Ei putkea argumentin --putki_ulos jälkeen\n");
	continue;
      }
      close(putki_ulos[0]);
      i+=2;
    }
  }
  signal(SIGINT, sigint_f);
  alusta_aani(SND_PCM_STREAM_CAPTURE);
  pthread_t saie;
  pit_data = taaj*tallennusaika_ms/1000;
  pit_jakso = taaj*jaksonaika_ms/1000;
  alku_kpl = pit_jakso + gauss_sigma_kpl*6 + 1 + maksimin_alue*2;
  suod_kpl = alku_kpl - gauss_sigma_kpl*6;
  deri_kpl = suod_kpl - 1;
  ohen_kpl = suod_kpl - maksimin_alue*2; // == pit_jakso
  float *data[6];
  for(int i=0; i<2; i++)
    data[i] = calloc(pit_jakso*sizeof(float), 1);
  data[raaka] = calloc(pit_data*sizeof(float), 1);
  data[suodate] = calloc(suod_kpl*sizeof(float), 1);
  data[derivoitu] = calloc(deri_kpl*sizeof(float), 1);
  data[ohennus] = calloc(ohen_kpl*sizeof(float), 1);
  pthread_create(&saie, NULL, nauhoita, data);
  kasittele(data);
  pthread_join(saie, NULL);
  snd_pcm_close(kahva_capt);
  for(int i=0; i<6; i++)
    free(data[i]);
  return 0;
}
