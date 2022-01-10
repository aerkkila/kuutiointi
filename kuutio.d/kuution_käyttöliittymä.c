/*Tämä liitetään include-komennolla pääfunktioon*/

#define siirtoInl1(tahko, maara) siirtoInl(tahko, siirtokaista, maara)
SDL_Event tapaht; 
int xVanha=0, yVanha=0; //alustetaan, ettei kääntäjä varoittele turhaan
char hiiri_painettu = 0;
int siirtokaista = 1;
int raahattiin = 0;
int vaihto = 0;
int numero1_pohjassa = 0;
int numero10_pohjassa = 0;
TOISTOLAUSE:
while(SDL_PollEvent(&tapaht)) {
  switch(tapaht.type) {
  case SDL_QUIT:
    goto ULOS;
  case SDL_WINDOWEVENT:
    switch(tapaht.window.event) {
    case SDL_WINDOWEVENT_RESIZED:;
      int koko1 = (tapaht.window.data1 < tapaht.window.data2)? tapaht.window.data1: tapaht.window.data2;
      kuva.xRes = tapaht.window.data1;
      kuva.yRes = tapaht.window.data2;
      for(int i=0; i<2; i++) {
	SDL_DestroyTexture(alusta[i]);
	if(!(alusta[i] = SDL_CreateTexture(kuva.rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, kuva.xRes, kuva.yRes)))
	  printf("Ei tehty alustaa %i\n", i);
	SDL_SetTextureBlendMode(alusta[i], SDL_BLENDMODE_BLEND);
      }
      kuva.resKuut = koko1/sqrt(3.0);
      if(kuva.resKuut/kuutio.N*kuva.mustaOsuus < 1)
	if(1/(kuva.resKuut/kuutio.N) < 0.5)
	  kuva.mustaOsuus = 1/(kuva.resKuut/kuutio.N);
	  
      if((koko1-kuva.resKuut)/2 != kuva.sij0) {
	kuva.sij0 = (koko1-kuva.resKuut)/2;
	tee_ruutujen_koordtit();
	kuva.paivita = 1;
      }
      break;
    }
    break;
  case SDL_KEYDOWN:
    switch(tapaht.key.keysym.scancode) {
    case SDL_SCANCODE_I:
      siirtoInl1(_u, 1);
      break;
    case SDL_SCANCODE_L:
      siirtoInl1(_r, 1);
      break;
    case SDL_SCANCODE_J:
      siirtoInl1(_r, 3);
      break;
    case SDL_SCANCODE_PERIOD:
      siirtoInl1(_d, 3);
      break;
    case SDL_SCANCODE_K:
      siirtoInl1(_f, 1);
      break;
    case SDL_SCANCODE_O:
      siirtoInl1(_b, 3);
      break;
    case SDL_SCANCODE_E:
      siirtoInl1(_u, 3);
      break;
    case SDL_SCANCODE_F:
      siirtoInl1(_l, 1);
      break;
    case SDL_SCANCODE_S:
      siirtoInl1(_l, 3);
      break;
    case SDL_SCANCODE_X:
      siirtoInl1(_d, 1);
      break;
    case SDL_SCANCODE_D:
      siirtoInl1(_f, 3);
      break;
    case SDL_SCANCODE_W:
      siirtoInl1(_b, 1);
      break;
    case SDL_SCANCODE_U:
      for(int kaista=2; kaista<N; kaista++)
	siirtoInl(_r, kaista, 1);
      break;
    case SDL_SCANCODE_R:
      for(int kaista=2; kaista<N; kaista++)
	siirtoInl(_l, kaista, 1);
      break;
      /*käytetään kääntämisten määrässä siirtokaistaa*/
    case SDL_SCANCODE_H:
      kaantoInl('y', (3*(siirtokaista)) % 4);
      break;
    case SDL_SCANCODE_G:
      kaantoInl('y', (1*(siirtokaista)) % 4);
      break;
    case SDL_SCANCODE_N:
      kaantoInl('x', (1*(siirtokaista)) % 4);
      break;
    case SDL_SCANCODE_V:
      kaantoInl('x', (3*(siirtokaista)) % 4);
      break;
    case SDL_SCANCODE_COMMA:
      kaantoInl('z', (1*(siirtokaista)) % 4);
      break;
    case SDL_SCANCODE_C:
      kaantoInl('z', (3*(siirtokaista)) % 4);
      break;
    default:
      switch(tapaht.key.keysym.sym) {
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
	siirtokaista++;
	vaihto = 1;
	break;
      case SDLK_PAUSE:
	if(vaihto)
	  asm("int $3");
	break;
      case SDLK_RETURN:
	kaantoanimaatio(_f, 0, (koordf){{0,1,0}}, 1.0, 1);
	break;
#define A kuva.ruutuKorostus
#define B(i) kuva.ruutuKorostus.a[i]
      case SDLK_LEFT:
	A = hae_ruutu(kuutio.N, B(0), B(1)-1, B(2));
	kuva.paivita=1;
	break;
      case SDLK_RIGHT:
	A = hae_ruutu(kuutio.N, B(0), B(1)+1, B(2));
	kuva.paivita=1;
	break;
      case SDLK_UP:
	if(A.a[0] < 0)
	  A = (int3){{_f, kuutio.N/2, kuutio.N/2}};
	else if(vaihto)
	  A.a[0] = -1;
	else
	  A = hae_ruutu(kuutio.N, B(0), B(1), B(2)-1);
	kuva.paivita=1;
	break;
      case SDLK_DOWN:
	A = hae_ruutu(kuutio.N, B(0), B(1), B(2)+1);
	kuva.paivita=1;
	break;
#undef A
#undef B
#ifndef __EI_SEKUNTIKELLOA__
      case SDLK_F1:
	lue_siirrot(ipc);
	kuva.paivita = 1;
	ipc->viesti = ipcTarkastelu;
	viimeViesti = ipcTarkastelu;
	break;
      case SDLK_SPACE:
	if(viimeViesti == ipcAloita) {
	  ipc->viesti = ipcLopeta;
	  viimeViesti = ipcLopeta;
	} else {
	  ipc->viesti = ipcAloita;
	  viimeViesti = ipcAloita;
	}
	break;
      case SDLK_TAB:
	if(viimeViesti == ipcLopeta) {
	  ipc->viesti = ipcJatka;
	  viimeViesti = ipcAloita;
	}
	break;
#endif
#ifndef EI_SAVEL_MAKRO
      case SDLK_F2:; //käynnistää tai sammuttaa sävelkuuntelijan
	if(!savelPtr) {
	  pid_t pid1, pid2;
	  if((pid1 = fork()) < 0) {
	    perror("Haarukkavirhe pid1");
	    break;
	  } else if(!pid1) {
	    if((pid2 = fork()) < 0) {
	      perror("Haarukkavirhe pid2");
	      exit(1);
	    } else if (pid2) {
	      _exit(0);
	    } else {
	      if(system("./sävel.py") < 0)
		perror("sävel.py");
	      exit(0);
	    }
	  } else
	    waitpid(pid1, NULL, 0);
	  savelPtr = savelmuistiin();
	  *savelPtr = -1.0;
	  break;
	} else {
	  savelPtr = sulje_savelmuisti((void*)savelPtr);
	}
#endif
      default:
	if('1' <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9' && !numero1_pohjassa) {
	  numero1_pohjassa = 1;
	  siirtokaista += tapaht.key.keysym.sym - '1';
	} else if(SDLK_KP_1 <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= SDLK_KP_9 && !numero10_pohjassa) {
	  numero10_pohjassa = 1;
	  siirtokaista += (tapaht.key.keysym.sym - SDLK_KP_1 + 1) * 10;
	}
	break;
      }
      break;
    }
    break;
  case SDL_KEYUP:
    switch(tapaht.key.keysym.sym) {
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
      vaihto = 0;
      siirtokaista = 1;
      break;
    default:
      if('1' <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= '9') {
	numero1_pohjassa = 0;
	if(!vaihto)
	  siirtokaista = 1;
      } else if(SDLK_KP_1 <= tapaht.key.keysym.sym && tapaht.key.keysym.sym <= SDLK_KP_9) {
	numero10_pohjassa = 0;
	if(!vaihto)
	  siirtokaista = 1;
      }
      break;
    }
    break;
  case SDL_MOUSEBUTTONDOWN:
    xVanha = tapaht.button.x;
    yVanha = tapaht.button.y;
    hiiri_painettu = 1;
    raahattiin = 0;
    if(tapaht.button.button == SDL_BUTTON_RIGHT)
      hiiri_painettu = 2;
    break;
  case SDL_MOUSEMOTION:;
    /*pyöritetään, raahauksesta hiirellä*/
    if(!hiiri_painettu)
      break;
    raahattiin = 1;
    static double aika = NAN; //annetaan liikkeen kertyä lyhyen aikaa ennen pyörittämistä
    static float xEro = 0;
    static float yEro = 0;
    xEro += (tapaht.motion.x-xVanha) * PI/(2*kuva.resKuut); //vasemmalle negatiivinen
    yEro += (tapaht.motion.y-yVanha) * PI/(2*kuva.resKuut); //alas positiivinen
    xVanha = tapaht.motion.x;
    yVanha = tapaht.motion.y;
    if(aika != aika) {
      aika = hetkiNyt(); //laitetaan tämä odottamaan ajan kertymistä
      break;
    }
    if(hetkiNyt() - aika < 0.02)
      break;
    /*aikaa on kertynyt tarpeeksi*/
    aika = NAN;
    /*lähes vaaka- tai pystysuora liike tulkitaan kokonaan sellaiseksi,
      vinottain liike on huonosti määritelty, koska järjestyksellä on väliä*/
    float suhde = yEro / xEro;
    if(ABS(suhde) > 3)
      xEro = 0;
    else if(ABS(suhde) < 0.3333)
      yEro = 0;
    koordf akseli = {{[1] = xEro}};
    akseli.a[hiiri_painettu & 2] = yEro; // x- tai z-akseli
    xEro = 0; yEro = 0;
	
    for(int i=0; i<3; i++)
      kuva.kannat[i] = puorauta(kuva.kannat[i], akseli);
	  
    tee_ruutujen_koordtit();
    kuva.paivita = 1;
    break;
  case SDL_MOUSEBUTTONUP:
    hiiri_painettu = 0;
    if(!raahattiin) {
      int tahko = mika_tahko(tapaht.motion.x, tapaht.motion.y);
      if(tahko >= 0)
	siirtoInl(tahko, siirtokaista, (tapaht.button.button == 1)? 3: 1);
    }
    break;
  }
 } //while poll_event
#ifndef EI_SAVEL_MAKRO
if(savelPtr) {
  static char suunta = 1;
  static double savLoppuHetki = 1.0;
  float savel = *savelPtr;
  if(savel < 0) {
    if(savLoppuHetki < 0) { //sävel päättyi juuri
      struct timeval hetki;
      gettimeofday(&hetki, NULL);
      savLoppuHetki = hetki.tv_sec + hetki.tv_usec*1.0/1000000;
    } else if(savLoppuHetki > 2) { // 1 olisi merkkinä, ettei tehdä mitään
      struct timeval hetki;
      gettimeofday(&hetki, NULL);
      double hetkiNyt = hetki.tv_sec + hetki.tv_usec*1.0/1000000;
      if(hetkiNyt - savLoppuHetki > 1.0 && kuva.korostus >= 0) {
	siirtoInl1(kuva.korostus, suunta);
	kuva.korostus = -1;
	savLoppuHetki = 1;
	*savelPtr = -1.0;
      }
    }
    goto LOPPU;
  }
  savLoppuHetki = -1.0;
  int puoliask = savel_ero(savel);
  printf("%i\n", puoliask);
  *savelPtr = -1.0;
  suunta = 1;
  if(puoliask < 0) {
    puoliask += 12;
    suunta = 3;
  }
  switch(puoliask) {
  case 0:
    kuva.korostus = _r;
    break;
  case 2:
    kuva.korostus = _l;
    break;
  case 4:
    kuva.korostus = _u;
    break;
  case 5:
    kuva.korostus = _d;
    break;
  case 7:
    kuva.korostus = _f;
    break;
  case 10:
    kuva.korostus = _b;
    break;
  default:
    goto LOPPU;
  }
  kuva.paivita = 1;
 }
LOPPU:
#endif
if(kuva.paivita)
  paivita();
SDL_Delay(20);
goto TOISTOLAUSE;
