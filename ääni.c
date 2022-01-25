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
const int tallennusaika_ms = 1000;
const int jaksonaika_ms = 30;
const int gauss_sigma_kpl = 70;
const int maksimin_alue = 30; //yhteen suuntaan
int alku_kpl, suod_kpl, deri_kpl, ohen_kpl;
unsigned tarkka_taaj;
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
  const int raaka = 2, suodate = 3, derivoitu = 4, ohennus = 5;
  float** data = datav;
  int pit_data = taaj*tallennusaika_ms/1000;
  int pit_jakso = taaj*jaksonaika_ms/1000; // == ohen_kpl
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
    kumpaa_luetaan = -1; //merkki nauhoitusfunktiolle
    kumpi_valmis = -1; //merkki tälle funktiolle
    suodata(data[raaka]+pit_data-alku_kpl, data[suodate], alku_kpl, gauss_sigma_kpl);
    derivaatta(data[suodate], data[derivoitu], suod_kpl);
    ohentaminen(data[derivoitu], data[ohennus], deri_kpl, maksimin_alue);
    for(int i=0; i<ohen_kpl; i++) {
      float arvo = data[ohennus][i];
      if(arvo > 1.0e-5)
	printf("\033[31mYlittyi\033[0m: arvo = %.3e\n", arvo);
    }
  }
}

int main() {
  signal(SIGINT, sigint_f);
  alusta_aani();
  pthread_t saie;
  alku_kpl = taaj*jaksonaika_ms/1000 + gauss_sigma_kpl*6 + 1 + maksimin_alue*2;
  suod_kpl = alku_kpl - gauss_sigma_kpl*6;
  deri_kpl = suod_kpl - 1;
  ohen_kpl = suod_kpl - maksimin_alue*2;
  float *data[6];
  for(int i=0; i<2; i++)
    data[i] = malloc(taaj*sizeof(float)*jaksonaika_ms/1000);
  data[2] = malloc(taaj*sizeof(float)*tallennusaika_ms/1000); //raakadata
  data[3] = malloc(suod_kpl*sizeof(float)); //suodate
  data[4] = malloc(deri_kpl*sizeof(float)); //derivoitu
  data[5] = malloc(ohen_kpl*sizeof(float)); //ohennus
  pthread_create(&saie, NULL, nauhoita, data);
  kasittele(data);
  pthread_join(saie, NULL);
  snd_pcm_close(kahva);
  for(int i=0; i<6; i++)
    free(data[i]);
  return 0;
}
