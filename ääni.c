#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

snd_pcm_t* kahva;
const int taaj = 48000;
const int tallennusaika_ms = 400;
const int jaksonaika_ms = 30;
const int testiaika_ms = 30;
const int gauss_sigma_kpl = 7;
const int maksimin_liike = 22;
int tarkka_taaj;
snd_pcm_uframes_t buffer_size;
int virhe_id;
#define ALSAFUNK(fun,...) if( (virhe_id = fun(__VA_ARGS__)) < 0 )	\
    {									\
      fprintf(stderr,"Virhe funktiossa %s:\n%s\n",#fun,snd_strerror(virhe_id)); \
      return virhe_id;							\
    }

int alusta_aani() {
  snd_pcm_hw_params_t* hwparams;
  ALSAFUNK(snd_pcm_open, &kahva, "default", SND_PCM_STREAM_CAPTURE, 0);                        //avaa
  snd_pcm_hw_params_alloca(&hwparams);
  ALSAFUNK(snd_pcm_hw_params_any, kahva, hwparams);                                            //params_any
  ALSAFUNK(snd_pcm_hw_params_set_access, kahva, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);      //access
  ALSAFUNK(snd_pcm_hw_params_set_rate_resample, kahva, hwparams, 0);
  ALSAFUNK(snd_pcm_hw_params_set_format, kahva, hwparams, SND_PCM_FORMAT_FLOAT_LE);            //format
  tarkka_taaj = taaj;
  ALSAFUNK(snd_pcm_hw_params_set_rate_near, kahva, hwparams, &tarkka_taaj, 0);                 //rate
  if(taaj != tarkka_taaj)
    printf("Käytetään näytteenottotaajuutta %i taajuuden %i sijaan\n", tarkka_taaj, taaj);
  ALSAFUNK(snd_pcm_hw_params_set_channels, kahva, hwparams, 1);                                //channels 
  ALSAFUNK(snd_pcm_hw_params,kahva,hwparams);
  snd_pcm_hw_params_get_buffer_size(hwparams,&buffer_size);
  snd_pcm_prepare(kahva);
  return 0;
}

void siirra_dataa(float* data, int pit_data, int siirto) {
  for(int i=0; i<pit_data-siirto; i++)
    data[i] = data[i+siirto];
}

void patka_dataan(float* data, float* patka, int pit_data, int pit_patka) {
  siirra_dataa(data, pit_data, pit_patka);
  memcpy(data+pit_data-pit_patka, patka, pit_patka);
}

float mean2(float* suote, int pit) {
  float r = 0;
  for(int i=0; i<pit; i++)
    r += suote[i]*suote[i];
  return r/pit;
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

void eimaksimien_poisto(float* data, float* kohde, int pit) {
  for(int i=maksimin_liike; i<pit-maksimin_liike; i++)
    for(int j=1; j<=maksimin_liike; j++) {
      if( ! (data[i-j] < data[i] && data[i] > data[i+j]) ) {
	kohde[i-maksimin_liike] = 0;
	break;
      }
      kohde[i-maksimin_liike] = data[i];
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
    while(id == kumpaa_luetaan)
      usleep(1000);
    while(snd_pcm_readi( kahva, data[id], taaj*jaksonaika_ms/1000 ) < 0) {
      snd_pcm_prepare(kahva);
      fprintf(stderr, "Puskurin ylitäyttö\n");
    }
    kumpi_valmis = id;
    id = (id + 1) % 2;
  }
  return NULL;
}

void kasittele(void* datav) {
  const int raaka = 2, suodate = 3;
  float** data = datav;
  int pit_data = taaj*tallennusaika_ms/1000;
  int pit_jakso = taaj*jaksonaika_ms/1000;
  int pit_testi = taaj*testiaika_ms/1000;
  int pit_raaka = pit_jakso+gauss_sigma_kpl*6;
  while(nauh_jatka) {
    while(kumpi_valmis < 0)
      usleep(1000);
    int id = kumpi_valmis;
    kumpaa_luetaan = id;
    patka_dataan(data[raaka], data[id], pit_raaka, pit_jakso);
    kumpaa_luetaan = -1; //merkki nauhoitusfunktiolle
    kumpi_valmis = -1; //merkki tälle funktiolle
    siirra_dataa(data[suodate], pit_data, pit_jakso);
    suodata(data[raaka], data[suodate]+pit_data-pit_jakso, pit_raaka, gauss_sigma_kpl);
    float* ptr = data[suodate]+pit_data-pit_jakso-1;
    derivaatta(ptr, ptr, pit_jakso);
    eimaksimien_poisto(ptr-1, ptr-1+maksimin_liike, pit_jakso-1);
    float odote = mean2(data[suodate], pit_data-pit_testi-1);
    float arvo = mean2(data[suodate]+pit_data-pit_testi, pit_testi-1);
    if(arvo > odote*16)
      printf("\033[31mYlittyi\033[0m: odote=%.4e, arvo=%.3fodote\n", odote, arvo/odote);
  }
}

int main() {
  signal(SIGINT, sigint_f);
  alusta_aani();
  pthread_t saie;
  float *data[4];
  for(int i=0; i<2; i++)
    data[i] = malloc(taaj*sizeof(float)*jaksonaika_ms/1000);
  data[2] = malloc(taaj*sizeof(float)*jaksonaika_ms/1000 + gauss_sigma_kpl*6*sizeof(float)); //raakadata
  data[3] = malloc(taaj*sizeof(float)*tallennusaika_ms/1000); //suodate
  pthread_create(&saie, NULL, nauhoita, data);
  kasittele(data);
  pthread_join(saie, NULL);
  snd_pcm_close(kahva);
  for(int i=0; i<4; i++)
    free(data[i]);
  return 0;
}
