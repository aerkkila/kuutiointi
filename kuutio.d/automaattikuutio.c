#define KAANTO(tahko, kaista, maara, aika) {				\
    koordf akseli = kuva.kannat[tahko%3];				\
    if(tahko/3)								\
      for(int i=0; i<3; i++)						\
	akseli.a[i] *= -1;						\
    kaantoanimaatio(tahko, kaista, akseli, maara-2, aika);		\
    siirto(&kuutio,tahko,kaista,maara);					\
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
    KAANTO(_r, 1, 1, aika);
    break;
  case 1:
    KAANTO(_b, 1, 1, aika);
    break;
  case 2:
    KAANTO(_l, 1, 1, aika);
    break;
  case 3:
    KAANTO(_f, 1, 1, aika);
    break;
  }
  
  kuutio.ratkaistu = onkoRatkaistu(&kuutio);
  autolasku++;
 } while(!kuutio.ratkaistu);

ipc->viesti = ipcLopeta;
viimeViesti = ipcLopeta;
paivita();
printf("%i siirtoa\n", autolasku);
