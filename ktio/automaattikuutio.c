/*lisätään #include-komennolla tiedoston kuutio.c pääfunktioon*/
  
#define KAANTO(tahko, kaista, maara, aika) {				\
    koordf akseli = kuva.kannat[tahko%3];				\
    if(tahko/3)								\
      for(int i=0; i<3; i++)						\
	akseli.a[i] *= -1;						\
    kaantoanimaatio(tahko, kaista, akseli, maara-2, aika);		\
    siirto(tahko,kaista,maara);						\
    tee_ruutujen_koordtit();						\
  }

static int autolasku = 0;
SDL_Event tapahtauto;
viimeViesti = 0;
paivita();
while(!viimeViesti) {
  while(SDL_PollEvent(&tapahtauto)) {
    switch(tapahtauto.type) {
    case SDL_QUIT:
      goto ULOS;
    case SDL_KEYUP:
      switch(tapahtauto.key.keysym.sym) {
      case SDLK_SPACE:
	ipc->viesti = ipcAloita;
	viimeViesti = ipcAloita;
	break;
      }
      break;
    }
  }//pollEvent
 } //aloitussilmukka

do {
  while(SDL_PollEvent(&tapahtauto)) {
    switch(tapahtauto.type) {
    case SDL_QUIT:
      goto ULOS;
    }
  }//pollEvent
#include "automaattisiirrot.c" //tänne ohjelmoidaan siirrot ja viive
  kuutio.ratkaistu = onkoRatkaistu();
  autolasku++;
 } while(!kuutio.ratkaistu);

ipc->viesti = ipcLopeta;
viimeViesti = ipcLopeta;
paivita();
printf("%i siirtoa\n", autolasku);
