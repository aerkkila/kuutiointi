#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
//#include <flista.h>
//#include <strlista.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "grafiikka.h"
#include "tulokset.h"
#include "muistin_jako.h"
#include "asetelma.h"
#include <math.h>
//#include <lista_math.h>

typedef enum {
  kelloal,
  tuloksetal,
  jarjestus1al,
  jarjestus2al,
  tiedotal,
  sektusal,
  tarkasteluaikanappial,
  tietoalue,
  lisatdal,
  muutal,
  muual
} alue_e;

char piste_alueella(int x, int y, SDL_Rect* alue);
alue_e hae_alue(int x, int y);
char* sekoitus(char* s);
void laita_eri_sekunnit(char* tmp);
void vaihda_fonttikoko(tekstiolio_s* olio, int y);
inline void __attribute__((always_inline)) laita_sekoitus(shmRak_s* ipc, char* sek);
inline char __attribute__((always_inline)) rullaustapahtuma_alusta(tekstiolio_s*, SDL_Event);
inline char __attribute__((always_inline)) rullaustapahtuma_lopusta(tekstiolio_s*, SDL_Event);

#define KELLO (kellool.teksti)
#define TEKSTI (tkstalol.teksti)
#define HETKI tuloshetki
#define MUUTA_TULOS laitot |= muuta_tulos;
#define LISTARIVI(nimi, tapahtlaji) ((nimi).alku +			\
				     (tapaht.tapahtlaji.y - (nimi).toteutuma.y) / \
				     TTF_FontLineSkip((nimi).font))
#define TEE_TIEDOT tiedot = tee_tiedot(tiedot, avgind);
#define KIRJOITUSLAJIKSI(laji) {			   \
    SDL_StartTextInput();				   \
    SDL_SetTextInputRect(&kellool.sij);			   \
    tila = kirjoitustila;				   \
    kirjoituslaji = laji;				   \
    nostotoimi = ei_mitaan;				   \
    strcpy(KELLO, "");					   \
    laitot |= kellolai;					   \
    strcpy(TEKSTI, tekstialue[kirjoituslaji]);		   \
    laitot |= tkstallai;					   \
  }

extern float skaala;
shmRak_s* ipc;

int kaunnista() {
  SDL_Event tapaht;
  struct timeval alku, nyt;
  short min, sek, csek;
  double dalku, dnyt;
  char tmp[1000];
  int avgind[6];
  int apuind;
  enum hiirilaji {
    perus,
    teksti,
    kasi
  } hlaji = perus;
  enum {
    ei_mitaan,
    aloita,
    tarkastelu
  } nostotoimi;
  enum {
    seis,
    tarkastelee,
    juoksee,
    kirjoitustila
  } tila = seis;
  enum {
    aikaKirj,
    ulosnimiKirj,
    tulosalkuKirj,
    avaa_tiedostoKirj,
    kuutionKokoKirj,
    karsintaKirj
  } kirjoituslaji = aikaKirj;
  char* tekstialue[] = {"Ajan syöttö",			\
			"Ulosnimen vaihto",		\
			"Tuloslistan alkukohta",	\
			"Avattava tiedosto",		\
			"Kuution koko (NxNxN)",		\
			"Keskiarvon karsinta"};
  char kontrol = 0;
  ipc = NULL;
  nostotoimi = (tarknap.valittu)? tarkastelu : aloita;
  alue_e alue = muual;
  sakko_e sakko;
  SDL_Cursor* kursori;
  kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  SDL_SetCursor(kursori);

  char* apucp;

  TEE_TIEDOT;
  SDL_StopTextInput();
  slistalle_kopioiden(sektus, sekoitus(tmp));
  while(1) {
    while(SDL_PollEvent(&tapaht)) {
      switch(tapaht.type)
	{
	case SDL_QUIT:
	  SDL_FreeCursor(kursori);
	  return 0;
	case SDL_KEYDOWN:
	  switch(tapaht.key.keysym.sym)
	    {
	    case SDLK_SPACE:
	    LOPETA:
	      if(tila == juoksee) {
		tila = seis;
		lisaa_listoille(KELLO, nyt.tv_sec);
		slistalle_kopioiden(sektus, sekoitus(tmp));
		TEE_TIEDOT;
		MUUTA_TULOS;
	      }
	      break;
	    case SDLK_LCTRL:
	    case SDLK_RCTRL:
	      kontrol = 1;
	      break;
	    case SDLK_o:
	      if(tila == seis)
		KIRJOITUSLAJIKSI(avaa_tiedostoKirj);
	      break;
	    case SDLK_s:
	      if(tila != seis)
		break;
	      if(kontrol) {
		sprintf(tmp, "%s/%s", uloskansio, ulosnimi);
		if(tallenna(tmp))
		  sprintf(TEKSTI, "Tallennettiin \"%s\"", ulosnimi);
		else
		  sprintf(TEKSTI, "Ei tallennettu \"%s\"", ulosnimi);
		laitot |= tkstallai;
	      } else { //s ilman ctrl:ia, vaihdetaan ulosnimi
	        KIRJOITUSLAJIKSI(ulosnimiKirj);
	      }
	      break;
	    case SDLK_BACKSPACE:
	      if(tila == seis && strtulos) {
		poista_listoilta_viimeinen();
		if(stulos->pit)
		  strcpy(KELLO, *VIIMEINEN(stulos));
		TEE_TIEDOT;
	        MUUTA_TULOS;
	      } else if (tila == kirjoitustila) {
		/*koko utf-8-merkki pois kerralla*/
		char jatka = 1;
		while(jatka) {
		  jatka = ( (KELLO[strlen(KELLO)-1] & 0xc0) == 0x80 )? 1 : 0; //alkaako 10:lla
		  KELLO[strlen(KELLO)-1] = '\0';
		}
	      }
	      laitot |= kellolai;
	      break;
	    case SDLK_RETURN:
	    case SDLK_KP_ENTER:
	      SDL_RenderClear(rend);
	      laitot = kaikki_laitot;
	      if(tila == kirjoitustila) {
		SDL_StopTextInput();
	        tila = seis;
	        nostotoimi = (tarknap.valittu)? tarkastelu : aloita;
		switch((int)kirjoituslaji) {
		case aikaKirj:
		  /*tällä voi kysyä SDL-version*/
		  if(!strcmp("SDL -v", KELLO)) {
		    SDL_version v;
		    SDL_VERSION(&v);
		    sprintf(KELLO, "%hhu.%hhu.%hhu", v.major, v.minor, v.patch);
		    laitot |= kellolai;
		    break;
		  }
		  /*laitetaan tuloksiin se, mitä kirjoitettiin*/
		  while((apucp = strstr(KELLO, ".")))
		    *apucp = ',';
		  lisaa_listoille(KELLO, time(NULL));
		  TEE_TIEDOT;
		  slistalle_kopioiden(sektus, sekoitus(tmp));
		  MUUTA_TULOS;
		  break;
		case ulosnimiKirj:
		  /*vaihdetaan ulosnimi ja kelloon taas aika*/
		  poista_slistalta_viimeinen(muut_b);
		  slistalle_kopioiden(muut_b, KELLO);
		  ulosnimi = muut_b->taul[0];
		  TEKSTI[0] = '\0';
		  break;
		case tulosalkuKirj:
		  sscanf(KELLO, "%i", &apuind);
		  if(apuind <= _ylaske_taakse(strtulos)) {
		    tulosol.rullaus += tulosol.alku - (apuind-1);
		    strcpy(TEKSTI, "");
		  } else {
		    strcpy(TEKSTI, "Hähää, eipäs onnistukaan");
		  }
		  break;
		case kuutionKokoKirj:
		  sscanf(KELLO, "%u", &NxN);
		  strcpy(TEKSTI, "");
		  poistia_slistalta_viimeinen(sektus);
		  slistalle_kopioiden(sektus, sekoitus(tmp));
		  break;
		case avaa_tiedostoKirj:
		  lue_tiedosto(KELLO, "");
		  TEE_TIEDOT;
		  laitot = kaikki_laitot;
		  break;
		case karsintaKirj:
		  sscanf(KELLO, "%u", &karsinta);
		  strcpy(TEKSTI, "");
		  TEE_TIEDOT;
		  break;
		}
		/*koskee kaikkia kirjoituslajeja*/
		if(ftulos)
		  float_kelloksi(KELLO, *VIIMEINEN(ftulos));
		break;
	      }
	    case SDLK_END:
	      switch(alue) {
	      default:
	      case tuloksetal:
		tulosol.rullaus = 0;
		laitot |= tuloslai;
		break;
	      case jarjestus1al:;
		int mahtuu = jarjol1.sij.h / TTF_FontLineSkip(jarjol1.font);
		jarjol1.rullaus = -(stulos->pit-1 - mahtuu);
		laitot |= jarjlai;
		break;
	      case jarjestus2al:
		jarjol2.rullaus = 0;
		laitot |= jarjlai;
		break;
	      case sektusal:
		sektusol.rullaus = 0;
		laitot |= sektuslai;
		break;
	      }
	      break;
	    case SDLK_HOME:
	      switch(alue) {
	      default:
	      case tuloksetal:
		tulosol.rullaus += tulosol.alku;
		laitot |= tuloslai;
		break;
	      case jarjestus1al:
		jarjol1.rullaus = 0;
		laitot |= jarjlai;
		break;
	      case jarjestus2al:
		jarjol2.rullaus += jarjol2.alku;
		laitot |= jarjlai;
		break;
	      case sektusal:
		sektusol.rullaus += sektusol.alku;
		laitot |= sektuslai;
		break;
	      }
	      break;
	    case SDLK_ESCAPE:
	      /*pois kirjoitustilasta muuttamatta mitään*/
	      if(tila == kirjoitustila) {
		SDL_StopTextInput();
		tila = seis;
		nostotoimi = (tarknap.valittu)? tarkastelu : aloita;
		if(strtulos)
		  strcpy(KELLO, *VIIMEINEN(stulos));
		TEKSTI[0] = '\0';
		laitot |= tkstallai;
		laitot |= kellolai;
	      }
	      break;
	    case SDLK_PLUS:
	    case SDLK_KP_PLUS:
	      if(kontrol) {
		SDL_RenderClear(rend);
		skaala *= 1.1;
		SDL_RenderSetScale(rend, skaala, skaala);
		laitot = kaikki_laitot;
	      } else if(tila != kirjoitustila) {
		muuta_sakko(KELLO, stulos->pit-1);
		TEE_TIEDOT;
		laitot |= kellolai;
		MUUTA_TULOS;
	      }
	      break;
	    case SDLK_MINUS:
	    case SDLK_KP_MINUS:
	      if(kontrol) {
		SDL_RenderClear(rend);
		skaala /= 1.1;
		SDL_RenderSetScale(rend, skaala, skaala);
		laitot = kaikki_laitot;
	      }
	      break;
	    }
	  break;
	case SDL_KEYUP:
	  switch(tapaht.key.keysym.sym)
	    {
	    case SDLK_SPACE:
	      switch(nostotoimi) {
	      case aloita:
	      ALOITA:
		if(tila != tarkastelee)
		  sakko = ei;
		gettimeofday(&alku, NULL);
	        nostotoimi = ei_mitaan;
	        tila = juoksee;
		kellool.vari = kellovarit[0];
		if(sakko==plus)
		  alku.tv_sec -= 2;
		strcpy(TEKSTI, "");
		break;
	      case tarkastelu:
	      TARKASTELU:
		sakko = ei;
		gettimeofday(&alku, NULL);
		dalku = alku.tv_sec + alku.tv_usec/1.0e6;
		nostotoimi = aloita;
		tila = tarkastelee;
		kellool.vari = kellovarit[1];
		strcpy(TEKSTI, "");
		break;
	      case ei_mitaan:
		if(tila != kirjoitustila) //pysäytetty juuri äsken
		  nostotoimi = (tarknap.valittu)? tarkastelu : aloita;
		    break;
	      }
	    case SDLK_LCTRL:
	    case SDLK_RCTRL:
	      kontrol = 0;
	      if(tila == kirjoitustila)
		strcpy(TEKSTI, tekstialue[kirjoituslaji]);
	      laitot |= tkstallai;
	      break;
	    }
	  break;
	case SDL_MOUSEBUTTONDOWN:
	  switch(alue) {
	  case kelloal:
	    if(tapaht.button.button == SDL_BUTTON_LEFT)
	      if(tila == seis)
		KIRJOITUSLAJIKSI(aikaKirj);
	    break;
	  case tietoalue:
	    if(tapaht.button.button == SDL_BUTTON_LEFT ||	\
	       tapaht.button.button == SDL_BUTTON_RIGHT) {
	      char rivi = ((tapaht.button.y - tluvutol.toteutuma.y) / \
			   TTF_FontLineSkip(tluvutol.font));
	      int sarake = ((tapaht.button.x - tluvutol.toteutuma.x) / \
			    (tluvutol.toteutuma.w / 6));
	      sarake -= (sarake/2 + 1); //nyt tämä on 0, 1 tai 2
	      tuhjenna_slista(lisatd);
	      slista* sektus1 = (tapaht.button.button == SDL_BUTTON_LEFT)? NULL : sektus;
	      switch (rivi) {
	      case 0:
	      case 1:
	        tuhjenna_slista(lisatd);
		tee_lisatiedot(sektus1, avgind[sarake]-4, 5);
		break;
	      case 2:
	      case 3:
	        tuhjenna_slista(lisatd);
		tee_lisatiedot(sektus1, avgind[sarake+3]-11, 12);
		break;
	      case 4:
	        tuhjenna_slista(lisatd);
		KIRJOITUSLAJIKSI(karsintaKirj);
		break;
	      }
	      if(tapaht.button.button == SDL_BUTTON_RIGHT) {
		int pit = 0;
		FOR_LISTA(lisatd)
		  pit += strlen(*NYT_OLEVA(lisatd))+1; //rivinvaihdosta tulee +1
		char tmp_oikea[pit+1];
		slista_sprintf(tmp_oikea, "%s\n", lisatd);
		SDL_SetClipboardText(tmp_oikea);
		lisatd = _strpoista_kaikki(lisatd);
	      } else {
		laitot |= lisatdlai;
	      }
	    }
	    break;
	  case tuloksetal:
	    if(tapaht.button.button == SDL_BUTTON_MIDDLE)
	      if(tila == seis)
		KIRJOITUSLAJIKSI(tulosalkuKirj);
	    break;
	  case sektusal:
	    if(tapaht.button.button == SDL_BUTTON_RIGHT)
	      KIRJOITUSLAJIKSI(kuutionKokoKirj);
	    break;
	  case muutal:;
	    int rivi = LISTARIVI(muutol, button);
	    if(rivi == _ylaske(muut_a))
	      rivi--;
	    char* tmpstr = muut_a->taul[rivi];
	    if(!strcmp(tmpstr, "ulosnimi:")) {
	      KIRJOITUSLAJIKSI(ulosnimiKirj);
	    } else if(!strcmp(tmpstr, "eri_sekunnit")) {
	      laita_eri_sekunnit(tmp);
	    } else if(!strcmp(tmpstr, "kuvaaja")) {
	      FILE *f = fopen(".kuvaaja.bin", "wb");
	      FOR_LISTA(ftulos)
		fwrite(NYT_OLEVA(ftulos), 1, sizeof(ftulos->taul[0]), f);
	      fclose(f);
	      pid_t pid1, pid2;
	      if( (pid1 = fork()) < 0 )
		fprintf(stderr, "Virhe: Ei tehty ensimmäistä alaprosessia\n");
	      else if(pid1) { //yläprosessi
		waitpid(pid1, NULL, 0);
	      } else { //1. alaprosessi
		if( (pid2 = fork()) < 0 ) {
		  fprintf(stderr, "Virhe: Ei tehty toista alaprosessia\n");
		  exit(1);
		} else if(pid2) { //1. alaprosessi
		  _exit(0);
		} else { //2. alaprosessi
		  system("python3 kuvaaja.py");
		  system("rm .kuvaaja.bin");
		  exit(0);
		}   
	      }
	    } else if(!strcmp(tmpstr, "kuutio")) {
	      /*avataan kuutio taustaprosessina*/
	      int pid1 = fork();
	      if(pid1 > 0)
		waitpid(pid1, NULL, 0);
	      else if(!pid1) {
		int pid2 = fork();
		if(pid2 > 0)
		  _exit(0);
		else if(!pid2) {
		  sprintf(tmp, "./kuutio %u", NxN);
		  system(tmp);
		  exit(0);
		}
	      }
	      ipc = liity_muistiin();
	      if(ipc)
		;//liput |= ipc_auki; //tarkista, onko kuutio auki
	    }
	    break;
	  default:
	    break;
	  }
	  break;
	case SDL_MOUSEBUTTONUP:
	  switch(alue) {
	  case tarkasteluaikanappial:
	    if(tapaht.button.button == SDL_BUTTON_LEFT) {
	      tarknap.valittu = (tarknap.valittu+1) % 2;
	      nostotoimi = (tarknap.valittu)? tarkastelu : aloita;
	      laitot |= vntalai;
	    }
	    break;
	    /*tulos- ja järjestysalue johtavat samaan toimintoon
	      järjestysalueilta luettu indeksi muunnetaan tulosalueen indeksiksi*/
	  case tuloksetal:
	    apuind = LISTARIVI(tulosol, button);
	    if(apuind == stulos->pit)
	      apuind--;
	    goto MBUP_TULOKSIA;
	  case jarjestus1al:
	    apuind = LISTARIVI(jarjol1, button);
	    goto MBUP_JARJ2;
	  case jarjestus2al:
	    apuind = LISTARIVI(jarjol2, button);
	  MBUP_JARJ2:
	    if(apuind+1 == _ylaske(sijarj))
	      break;
	    sscanf(((strlista*)_ynouda(sijarj, apuind+1))->str, "%i", &apuind);
	    apuind--;
	  MBUP_TULOKSIA:
	    if(tapaht.button.button == SDL_BUTTON_LEFT) {
	      if(kontrol) {
		/*poistetaan (ctrl + hiiri1)*/
		poista_listoilta(apuind);
		_strpoista1(_ynouda(_yalkuun(sektus), apuind), 1);
		TEE_TIEDOT;
		alue = hae_alue(tapaht.button.x, tapaht.button.y);
	      } else {
		/*kopioidaan leikepöydälle (hiiri1)*/
	        char* tmpstr = ((strlista*)_ynoudaf(strtulos, apuind, 0))->str;
		time_t aika_t = ((ilista*)_ynoudaf(HETKI, apuind, 0))->i;
		struct tm *aika = localtime(&aika_t);
		strftime(TEKSTI, 150, "%A %d.%m.%Y klo %H.%M", aika);
		/*sekoituksia ei tallenneta, joten tätä ei välttämättä ole*/
		strlista *tmpsektlis = _ynoudaf(sektus, apuind, 0);
		if(tmpsektlis) {
		  sprintf(tmp, "%s; %s\n%s", tmpstr, TEKSTI, tmpsektlis->str);
		  SDL_SetClipboardText(tmp);
		}
	      }
	    } else if (tapaht.button.button == SDL_BUTTON_RIGHT) {
	      strlista* tmpstr = _ynouda(_yalkuun(strtulos), apuind);
	      muuta_sakko((strtulos == tmpstr)? KELLO : tmp, apuind);
	      TEE_TIEDOT;
	    }
	    MUUTA_TULOS;
	    laitot |= kellolai;
	    break;
	  default:
	    break;
	  }
	  break;
	case SDL_MOUSEWHEEL:
	  if(kontrol) {
	    switch(alue) {
	    case kelloal:
	      vaihda_fonttikoko(&kellool, tapaht.wheel.y*4);
	      laitot |= kellolai;
	      break;
	    case tuloksetal:
	      vaihda_fonttikoko(&tulosol, tapaht.wheel.y);
	      laitot |= tuloslai;
	      break;
	    case sektusal:
	      vaihda_fonttikoko(&sektusol, tapaht.wheel.y);
	      laitot |= sektuslai;
	      break;
	    case jarjestus1al:
	    case jarjestus2al:
	      vaihda_fonttikoko(&jarjol1, tapaht.wheel.y);
	      jarjol2.font = jarjol1.font;
	      laitot |= jarjlai;
	      break;
	    case lisatdal:
	      vaihda_fonttikoko(&lisaol, tapaht.wheel.y);
	      laitot |= lisatdlai;
	      break;
	    default:
	      break;
	    }
	  } else {
	    switch(alue) {
	    case tuloksetal:
	      if(rullaustapahtuma_lopusta(&tulosol, tapaht))
		laitot |= tuloslai;
	      break;
	    case sektusal:
	      if(rullaustapahtuma_lopusta(&sektusol, tapaht))
		laitot |= sektuslai;
	      break;
	    case jarjestus1al: //laitetaan alusta, joten rullaus ≤ 0
	      if(rullaustapahtuma_alusta(&jarjol1, tapaht))
		laitot |= jarjlai;
	      break;
	    case jarjestus2al:
	      if(rullaustapahtuma_lopusta(&jarjol2, tapaht))
		laitot |= jarjlai;
	      break;
	    case lisatdal:
	      if(rullaustapahtuma_alusta(&lisaol, tapaht))
		laitot |= lisatdlai;
	      break;
	    case muutal:
	      if(rullaustapahtuma_alusta(&muutol, tapaht))
		laitot |= muutlai;
	      break;
	    default:
	      break;
	    }
	    break;
	  }
	case SDL_MOUSEMOTION:;
	  alue_e vanha = alue;
	  alue = hae_alue(tapaht.motion.x, tapaht.motion.y);
	  switch(alue) {
	  case kelloal:
	  case lisatdal:
	  case sektusal:
	    if(hlaji != teksti) {
	      SDL_FreeCursor(kursori);
	      kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	      SDL_SetCursor(kursori);
	      hlaji = teksti;
	    }
	    break;
	  case jarjestus1al:
	    apuind = LISTARIVI(jarjol1, motion);
	  case jarjestus2al:;
	    if(alue != jarjestus1al)
	      apuind = LISTARIVI(jarjol2, motion);
	    if(apuind+1 < _ylaske(sijarj)) {
	      sscanf(((strlista*)_ynouda(sijarj, apuind+1))->str, "%i", &apuind);
	      apuind--;
	      goto LAITA_AIKA_NAKUVIIN;
	    }
	    laitot |= tkstallai;
	    break;
	  case tuloksetal:;
	    /*laitetaan aika näkyviin*/
	    apuind = LISTARIVI(tulosol, motion);
	    if(apuind < _ylaske_taakse(tuloshetki)) {
	    LAITA_AIKA_NAKUVIIN:;
	      time_t aika_t = ((ilista*)_ynoudaf(HETKI, apuind, 0))->i;
	      struct tm *aika = localtime(&aika_t);
	      strftime(TEKSTI, 150, "%A %d.%m.%Y klo %H.%M.%S", aika);
	      laitot |= tkstallai;
	    }
	    /*ei break-komentoa*/
	  case muutal:
	  case tietoalue:
	  case tarkasteluaikanappial:
	    if(hlaji != kasi) {
	      SDL_FreeCursor(kursori);
	      kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	      SDL_SetCursor(kursori);
	      hlaji = kasi;
	    }
	    break;
	  case tiedotal:
	  case muual:
	    if(hlaji != perus) {
	      SDL_FreeCursor(kursori);
	      kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	      SDL_SetCursor(kursori);
	      hlaji = perus;
	    }
	    break;
	  }
	  if( (vanha == tuloksetal && alue != tuloksetal) ||		\
	      (vanha == jarjestus1al && alue != jarjestus1al) ||		\
	      (vanha == jarjestus2al && alue != jarjestus2al) ) { //poistuttiin tuloksista
	    laitot |= tkstallai;
	    if(tila != kirjoitustila)
	      strcpy(TEKSTI, "");
	    else
	      strcpy(TEKSTI, tekstialue[kirjoituslaji]);
	  }
	  
	  break;
	case SDL_TEXTINPUT:
	  strcat(KELLO, tapaht.text.text);
	  laitot |= kellolai;
	  break;
	case SDL_WINDOWEVENT:
	  switch(tapaht.window.event) {
	  case SDL_WINDOWEVENT_RESIZED:
	    SDL_RenderClear(rend);
	    laitot = kaikki_laitot;
	    break;
	  }
	  break;
	} //switch(tapaht.type)
    } //while(SDL_PollEvent(&tapaht))
    if(!ipc || !(ipc->viesti))
      goto JUOKSU_YMS;

    /*SDL-tapahtumat päättyivät
      seuraavaksi tarkistetaan mahdollisen kuution tapahtumat*/ 
    switch(ipc->viesti) {
    case ipcAnna_sekoitus:
      laita_sekoitus(ipc, sektus->str);
      break;
    case ipcTarkastelu:
      ipc->viesti = 0;
      goto TARKASTELU;
    case ipcAloita:
      ipc->viesti = 0;
      goto ALOITA;
    case ipcLopeta:
      ipc->viesti = 0;
      goto LOPETA;
    }

  JUOKSU_YMS:
    if(tila == juoksee) {
      gettimeofday(&nyt, NULL);
      sek = (int)( (nyt.tv_sec + nyt.tv_usec/1.0e6) - (alku.tv_sec + alku.tv_usec/1.0e6) );
      min = sek / 60;
      sek %= 60;
      csek = (nyt.tv_usec - alku.tv_usec + 1000000)/10000 % 100;
      if(!min)
	sprintf(KELLO, "%s%hi,%hi%hi%s",				\
		(sakko==dnf)? "Ø(" : "", sek, csek/10, csek%10,		\
		(sakko==ei)? "" : ( (sakko==plus)? "+" : ")" ));
      else
	sprintf(KELLO, "%s%hi:%hi%hi,%hi%hi%s",				\
		(sakko==dnf)? "Ø(" : "",				\
		min, sek/10, sek%10, csek/10, csek%10,			\
		(sakko==ei)? "" : ( (sakko==plus)? "+" : ")" ));
      laitot |= kellolai;
    } else if (tila == tarkastelee) {
      gettimeofday(&nyt, NULL);
      dnyt = nyt.tv_sec + nyt.tv_usec/1.0e6;
      char aika = 15 - (char)(dnyt - dalku);
      if(aika > 0) {
	sprintf(KELLO, "%3hhi", aika);
      } else if(aika > -2) {
	sprintf(KELLO, " +2");
	sakko = plus;
	kellool.vari = kellovarit[2];
      } else {
	sprintf(KELLO, "DNF");
	sakko = dnf;
	kellool.vari = kellovarit[3];
      }
      laitot |= kellolai;
    }
    
    if(laitot)
      piirra();
    SDL_Delay(viive);
  } //while(1)
}

char piste_alueella(int x, int y, SDL_Rect* alue) {
  if (x < alue->x)
    return 0;
  if (x > alue->x + alue->w)
    return 0;
  if (y < alue->y)
    return 0;
  if (y > alue->y + alue->h)
    return 0;
  return 1;
}

alue_e hae_alue(int x, int y) {
  if(piste_alueella(x, y, &kellool.toteutuma))
    return kelloal;
  if(piste_alueella(x, y, &tulosol.toteutuma))
    return tuloksetal;
  if(piste_alueella(x, y, &jarjol1.toteutuma))
    return jarjestus1al;
  if(piste_alueella(x, y, &jarjol2.toteutuma))
    return jarjestus2al;
  if(piste_alueella(x, y, &tiedotol.toteutuma))
    return tiedotal;
  if(piste_alueella(x, y, &sektusol.toteutuma))
    return sektusal;
  if(piste_alueella(x, y, &tarknap.kuvat.sij))
    return tarkasteluaikanappial;
  if(piste_alueella(x, y, &muutol.toteutuma))
    return muutal;
  if(piste_alueella(x, y, &lisaol.toteutuma))
    return lisatdal;
  if(piste_alueella(x, y, &tluvutol.toteutuma)) {
    if(((x - tluvutol.toteutuma.x) / (tluvutol.toteutuma.w / 6)) % 2)
      return tietoalue;
  }
  return muual;
}

char* sekoitus(char* s) {
  short pit;
  if(NxN == 2)
    pit = 9;
  else
    pit = (NxN-2)*20;
  unsigned paksuus = NxN/2;
  const char pinnat[] = "RLUDFBrludfb";
  const char suunnat[] = " '2";
  char akseli, viimeakseli=10, paks;
  int pinta, puolisko;
  char akselikielto=0, kieltoakseli=0;
  char puoliskokielto=0, sallittuPuolisko=0;
  int paksKieltoja[2];
  paksKieltoja[0] = 0;
  paksKieltoja[1] = 0;
  char *paksKiellot[2];
  paksKiellot[0] = malloc(paksuus);
  paksKiellot[1] = malloc(paksuus);

  for (int i=0; i<pit; i++) {
    /*akseli*/
    akseli = rand() % (3-akselikielto); //akselikielto on 1 tai 0
    if (akselikielto && akseli >= kieltoakseli) akseli++;
    if(akseli != viimeakseli) {
      puoliskokielto = 0;
      akselikielto = 0;
      for(int i1=0; i1<2; i1++)
	paksKieltoja[i1] = 0;
    }
    viimeakseli = akseli;
    
    /*puolisko*/
    puolisko = (puoliskokielto)? sallittuPuolisko : rand() % 2;
    pinta = akseli*2 + puolisko;
    
    /*paksuus*/
    //esim N=3 --> % N/2=1 --> aina 0
    char paksind = rand() % (paksuus - paksKieltoja[puolisko]);
    /*haetaan oikea paksuus
      paksind on indeksi sallittujen paksuuksien joukossa*/
    char loutuneet = -1;
    paks = 1;
    while(1) {
      char loutui = 1;
      for(int j=0; j<paksKieltoja[puolisko]; j++)
	if(paksKiellot[puolisko][j] == paks) {
	  loutui = 0;
	  break;
	}
      if(loutui) //kys paksuus oli sallittu
	if(++loutuneet == paksind)
	  break;
      paks++;
    }
    if(paks > 1) pinta += 6; //esim R --> r jne.

    /*lisätään uudet kiellot*/
    paksKiellot[puolisko][paksKieltoja[puolisko]] = paks;
    paksKieltoja[puolisko]++;
    /*Parillisilla kuutioilla ei sallita esim N/2u ja N/2d peräkkäin*/
    if(NxN % 2 == 0 && paks == paksuus) {
      int toinen = (puolisko+1) % 2;
      paksKiellot[toinen][paksKieltoja[toinen]] = paksuus;
      paksKieltoja[toinen]++;
    }
    for(int j=0; j<2; j++)
      if(paksKieltoja[j] >= paksuus) {
	if(puoliskokielto) {
	  akselikielto = 1;
	  kieltoakseli = akseli;
	} else {
	  puoliskokielto = 1;
	  sallittuPuolisko = (j+1) % 2;
	}
      }
    
    /*tulostus*/
    if(NxN > 5 && paks)
      if(i==0) {
	sprintf(s, "%u%c%c", paks, pinnat[pinta], suunnat[rand() % 3]);
      } else {
	sprintf(s, "%s %u%c%c", s, paks, pinnat[pinta], suunnat[rand() % 3]);
      }
    else
      if(i==0) {
	sprintf(s, "%c%c", pinnat[pinta], suunnat[rand() % 3]);
      } else {
	sprintf(s, "%s %c%c", s, pinnat[pinta], suunnat[rand() % 3]);
      }
  }

  free(paksKiellot[0]);
  free(paksKiellot[1]);
  return s;
}

inline void __attribute__((always_inline)) laita_eri_sekunnit(char* tmps) {
  int *ia = eri_sekunnit(fjarj->seur, NULL, 0);
  int tmp=0;
  int hyv = _ylaske(fjarj->seur);
  int yht = _ylaske(_yalkuun(ftulos));
  int dnf = yht - hyv;
  lisatd = _strpoista_kaikki(_yalkuun(lisatd));
  lisatd = _strlisaa_kopioiden(lisatd, "aika  määrä");
  float osuus;
  float kertuma = 0;
  while(ia[tmp] != -1) {
    osuus = ia[tmp+1]/(float)yht;
    kertuma += osuus;
    sprintf(tmps, "%i    %i    %.3f    %.3f",		\
	    ia[tmp], ia[tmp+1], osuus, kertuma);
    lisatd = _strlisaa_kopioiden(lisatd, tmps);
    tmp+=2;
  }
  if(dnf) {
    osuus = dnf/(float)yht;
    kertuma += osuus;
    sprintf(tmps, "DNF  %i    %.3f    %.3f", dnf, osuus, kertuma);
    lisatd = _strlisaa_kopioiden(lisatd, tmps);
  }
  free(ia);
  lisatd = _yalkuun(lisatd);
  laitot |= lisatdlai;
  return;
}

inline void __attribute__((always_inline)) vaihda_fonttikoko(tekstiolio_s* olio, int y) {
  TTF_CloseFont(olio->font);
  olio->font = NULL;
  olio->fonttikoko += y;
  olio->font = TTF_OpenFont(olio->fonttied, olio->fonttikoko);
  if(!olio->font)
    fprintf(stderr, "Virhe: Ei avattu fonttia uudesti: %s\n", TTF_GetError());
  return;
}

inline void __attribute__((always_inline)) laita_sekoitus(shmRak_s* ipc, char* sek) {
  strcpy(ipc->data, sek);
  ipc->viesti = 0;
}

char rullaustapahtuma_alusta(tekstiolio_s* o, SDL_Event tapaht) {
  int riveja = o->toteutuma.h / TTF_FontLineSkip(o->font);
  if((o->alku + riveja == _ylaske(sijarj)-1 && tapaht.wheel.y < 0) ||	\
     (o->rullaus == 0 && tapaht.wheel.y > 0))
    return 0;
  o->rullaus += tapaht.wheel.y;
  return 1;
}

char rullaustapahtuma_lopusta(tekstiolio_s* o, SDL_Event tapaht) {
  if((o->alku == 0 && tapaht.wheel.y > 0) ||	\
     (o->rullaus == 0 && tapaht.wheel.y < 0))
    return 0;
  o->rullaus += tapaht.wheel.y;
  return 1;
}
