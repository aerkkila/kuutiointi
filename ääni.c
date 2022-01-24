#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

snd_pcm_t* kahva;
const int taaj = 48000;
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

struct nauh_args {
  unsigned char** data;
  int data_id;
  int pit;
  int valmis;
};
int nauh_jatka = 1;

void* nauhoita(void* voidargs) {
  struct nauh_args* volatile args = voidargs;
  while( (volatile int)nauh_jatka ) {
    while(snd_pcm_readi( kahva, args->data[args->data_id], args->pit ) < 0) {
      snd_pcm_prepare(kahva);
      fprintf(stderr, "Puskurin ylitäyttö\n");
    }
    args->valmis = 1;
  }
  return NULL;
}

void kasittele(struct nauh_args* volatile args) {
 ALKU:
  args->valmis = 0;
  while(!args->valmis)
    usleep(1000); // 1 ms
  int id0 = args->data_id;
  args->data_id = (args->data_id + 1) % 2;
  goto ALKU;
}

int main() {
  alusta_aani();
  int pituus = taaj*0.03; //0,03 s
  if(pituus > buffer_size)
    ; //tämä pitäisi käsitellä
  pthread_t saie;
  unsigned char *data[2];
  for(int i=0; i<2; i++)
    data[i] = malloc(pituus*sizeof(float)+1000);
  struct nauh_args args = {.data=data, .pit=pituus};
  pthread_create(&saie, NULL, nauhoita, &args);
  kasittele(&args);
  pthread_join(saie, NULL);
  snd_pcm_close(kahva);
  free(data[0]);
  free(data[1]);
  return 0;
}
