#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

snd_pcm_t* kahva;
const int taaj = 48000;
int tallennusaika_ms = 1500; //millisekuntia
int jaksonaika_ms = 30;
int tarkka_taaj;
const int periods = 2;
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

void patka_dataan(float* data, float* patka, int pit_data, int pit_patka) {
  for(int i=0; i<pit_data-pit_patka; i++)
    data[i] = data[i+pit_patka];
  memcpy(data+pit_data-pit_patka, patka, pit_patka);
}

float mean2(float* suote, int pit) {
  float r = 0;
  for(int i=0; i<pit; i++)
    r += suote[i]*suote[i];
  return r/pit;
}

int pit;
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
    while(snd_pcm_readi( kahva, data[id], pit ) < 0) {
      snd_pcm_prepare(kahva);
      fprintf(stderr, "Puskurin ylitäyttö\n");
    }
    kumpi_valmis = id;
    id = (id + 1) % 2;
  }
  return NULL;
}

void kasittele(void* datav) {
  float** data = datav;
  int pit_data = taaj*tallennusaika_ms/1000;
  int pit_patka = taaj*jaksonaika_ms/1000;
  while(nauh_jatka) {
    while(kumpi_valmis < 0)
      usleep(1000);
    int id = kumpi_valmis;
    kumpaa_luetaan = id;
    kumpi_valmis = -1;
    float odote = mean2(data[2], pit_data);
    float arvo = mean2(data[id], pit_patka);
    if(arvo > odote*10)
      printf("Ylittyi: odote=%.4e, arvo=%.4e\n", odote, arvo);
    patka_dataan(data[2], data[id], pit_data, pit_patka);
  }
}

int main() {
  signal(SIGINT, sigint_f);
  alusta_aani();
  pit = taaj*jaksonaika_ms/1000;
  if(pit > buffer_size)
    ; //tämä pitäisi käsitellä
  pthread_t saie;
  float *data[3];
  for(int i=0; i<2; i++)
    data[i] = malloc(pit*sizeof(float)+1000);
  data[2] = malloc(taaj*sizeof(float)*tallennusaika_ms/1000);  
  pthread_create(&saie, NULL, nauhoita, data);
  kasittele(data);
  pthread_join(saie, NULL);
  snd_pcm_close(kahva);
  free(data[0]);
  free(data[1]);
  puts("\nLopetettiin asianmukaisesti");
  return 0;
}
