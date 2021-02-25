#include "ääni.h"
#include <stdio.h>
#include <string.h>
#include <SDL.h>

strlista* nauhoituslaitteet(strlista* l) {
  if(!SDL_WasInit(SDL_INIT_AUDIO))
    if(SDL_InitSubSystem(SDL_INIT_AUDIO))
      fprintf(stderr, "Äänen alustaminen epäonnistui %s\n", SDL_GetError());
  char apu[200];
  if(SDL_VERSION_ATLEAST(2,0,8)) {
    int maara = SDL_GetNumAudioDevices(1);
    for(int i=0; i<maara; i++) {
      const char* c = SDL_GetAudioDeviceName(i,1);
      if(!c) {
	sprintf(apu, "Ei luettu äänilaitteen nimeä: %s", SDL_GetError());
	l = _strlisaa_kopioiden(l, apu);
      } else
	l = _strlisaa_kopioiden(l, c);
    }
    sprintf(apu, "Nauhoituslaitteita %i", maara);
    l = _strlisaa_kopioiden(l, apu);
    return l;
  }
  SDL_version v;
  SDL_VERSION(&v);
  char viesti[] = "SDL-versio on ";
  sprintf(apu, "%s%hhu.%hhu.%hhu", viesti, v.major, v.minor, v.patch);
  l = _strlisaa_kopioiden(l, apu);
  l = _strlisaa_kopioiden(l, "Pitäisi olla vähintään 2.0.8");
  return l;
}

strlista* aaniajurit(strlista* l) {
  if(!SDL_WasInit(SDL_INIT_AUDIO))
    if(SDL_InitSubSystem(SDL_INIT_AUDIO))
      fprintf(stderr, "Äänen alustaminen epäonnistui %s\n", SDL_GetError());
  char apu[200];
  if(SDL_VERSION_ATLEAST(2,0,8)) {
    int maara = SDL_GetNumAudioDrivers();
    for(int i=0; i<maara; i++) {
      const char* c = SDL_GetAudioDriver(i);
      if(!c) {
	sprintf(apu, "Ei luettu ääniajurin nimeä: %s", SDL_GetError());
	l = _strlisaa_kopioiden(l, apu);
      } else
	l = _strlisaa_kopioiden(l, c);
    }
    sprintf(apu, "Ääniajuri nyt: %s", SDL_GetCurrentAudioDriver());
    l = _strlisaa_kopioiden(l, apu);
    return l;
  }
  SDL_version v;
  SDL_VERSION(&v);
  char viesti[] = "SDL-versio on ";
  sprintf(apu, "%s%hhu.%hhu.%hhu", viesti, v.major, v.minor, v.patch);
  l = _strlisaa_kopioiden(l, apu);
  l = _strlisaa_kopioiden(l, "Pitäisi olla vähintään 2.0.8");
  return l;
}

SDL_AudioDeviceID avaa_aani(const char* nimi, char* viesti) {
  SDL_AudioSpec halu, olo;
  SDL_AudioDeviceID laite;
  SDL_memset(&halu, 0, sizeof(halu));
  halu.freq = 48000;
  halu.format = AUDIO_F32;
  halu.channels = 1;
  halu.samples = 4096;

  laite = SDL_OpenAudioDevice(nimi, 1, &halu, &olo, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if(!laite)
    sprintf(viesti, "Ei avattu laitetta \"%s\"", nimi);
  else if(halu.channels != olo.channels)
    sprintf(viesti, "Saatiin %i kanavaa %i:n sijaan", olo.channels, halu.channels);
  return laite;
}
