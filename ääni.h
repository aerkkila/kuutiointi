#ifndef __AANI__
#define __AANI__

#include <listat.h>
#include <SDL.h>

strlista* nauhoituslaitteet(strlista* l);
strlista* aaniajurit(strlista* l);
SDL_AudioDeviceID avaa_aani(const char* nimi, char* viesti);

#endif
