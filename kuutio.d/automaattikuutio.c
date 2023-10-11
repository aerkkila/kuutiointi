#define KÄÄNTÖ(tahko, kaista, määrä, aika) {				\
    koordf akseli = kuva.kannat[tahko%3];				\
    if(tahko/3)								\
      for(int i=0; i<3; i++)						\
	akseli.a[i] *= -1;						\
    kaantoanimaatio(tahko, kaista, akseli, määrä-2, aika);		\
    siirto(&kuutio,tahko,kaista,määrä);					\
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

  double aika = 0.3;
  switch(autolasku%4) {
  case 0:
    KÄÄNTÖ(_r, 1, 1, aika);
    break;
  case 1:
    KÄÄNTÖ(_b, 1, 1, aika);
    break;
  case 2:
    KÄÄNTÖ(_l, 1, 1, aika);
    break;
  case 3:
    KÄÄNTÖ(_f, 1, 1, aika);
    break;
  }
  
  kuutio.ratkaistu = onkoRatkaistu(&kuutio);
  autolasku++;
 } while(!kuutio.ratkaistu);

ipc->viesti = ipcLopeta;
viimeViesti = ipcLopeta;
paivita();
printf("%i siirtoa\n", autolasku);
