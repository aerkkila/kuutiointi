#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <flista.h>
#include <strlista.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "rakenteet.h"
#include "grafiikka.h"
#include "tulokset.h"
#include "ääni.h"
#include "muistin_jako.h"
#include <math.h>
#include <lista_math.h>

typedef enum {
  kello,
  tulokset,
  jarjestus1,
  jarjestus2,
  tiedot,
  sektus,
  tarkasteluaikanappi,
  tietoalue,
  lisatd,
  muut,
  muu
} alue_e;

char piste_alueella(int x, int y, SDL_Rect* alue);
alue_e hae_alue(int x, int y, kaikki_s *kaikki);
char* sekoitus(char* s);
void laita_eri_sekunnit(kaikki_s* kaikki, char* tmp);
void vaihda_fonttikoko(tekstiolio_s* olio, int y);
inline void __attribute__((always_inline)) laita_sekoitus(shmRak_s* ipc, char* sek);

#define SEKTUS (kaikki->sekoitukset)
#define TIEDOT (kaikki->tiedot)
#define KELLO (kaikki->kello_o->teksti)
#define TEKSTI (kaikki->tkstal_o->teksti)
#define LAITOT (*(kaikki->laitot))
#define STRTULOS (kaikki->tkset->strtulos)
#define FTULOS (kaikki->tkset->ftulos)
#define HETKI (kaikki->tkset->tuloshetki)
#define SJARJ (kaikki->tkset->strjarj)
#define SIJARJ (kaikki->tkset->sijarj)
#define LISATD (kaikki->lisatd)
#define MUUTA_TULOS LAITOT.sektus=1; LAITOT.tulos=1; LAITOT.jarj=1; LAITOT.tiedot=1; LAITOT.lisatd=1
#define LISTARIVI(nimi) (kaikki->nimi->alku +			\
			 (tapaht.button.y - kaikki->nimi->toteutuma->y) / \
			 TTF_FontLineSkip(kaikki->nimi->font))
#define TEE_TIEDOT TIEDOT = tee_tiedot(TIEDOT, kaikki->tkset, avgind);
#define KIRJOITUSLAJIKSI(laji) {			   \
    SDL_StartTextInput();				   \
    SDL_SetTextInputRect(kaikki->kello_o->sij);		   \
    tila = kirjoitustila;				   \
    kirjoituslaji = laji;				   \
    nostotoimi = ei_mitaan;				   \
    strcpy(KELLO, "");					   \
    LAITOT.kello=1;					   \
    strcpy(TEKSTI, tekstialue[kirjoituslaji]);		   \
    LAITOT.tkstal = 1;					   \
  }

extern float skaala;
shmRak_s* ipc;

int kaunnista(kaikki_s *kaikki) {
  SDL_Event tapaht;
  struct timeval alku, nyt;
  short min, sek, csek;
  double dalku, dnyt;
  char tmp[1000];
  int avgind[6];
  int apuind;
  tekstiolio_s* o;
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
    aika,
    ulosnimi,
    tulosalku,
    avaa_tiedosto
  } kirjoituslaji = aika;
  char* tekstialue[] = {"Ajan syöttö",\
		       "Ulosnimen vaihto",\
			"Tuloslistan alkukohta",\
			"Avattava tiedosto"};
  char kontrol = 0;
  ipc = NULL;
  nostotoimi = (kaikki->vnta_o->valittu)? tarkastelu : aloita;
  alue_e alue = muu;
  sakko_e sakko;
  SDL_Cursor* kursori;
  kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  SDL_SetCursor(kursori);

  char* apucp;

  TEE_TIEDOT;
  SDL_StopTextInput();
  SEKTUS = _strlisaa_kopioiden(SEKTUS, sekoitus(tmp));
  while(1) {
    while(SDL_PollEvent(&tapaht)) {
      switch(tapaht.type)
	{
	case SDL_QUIT:
	  _strpoista_kaikki(_yalkuun(LISATD));
	  SDL_FreeCursor(kursori);
	  return 0;
	case SDL_KEYDOWN:
	  switch(tapaht.key.keysym.sym)
	    {
	    case SDLK_SPACE:
	    LOPETA:
	      if(tila == juoksee) {
		tila = seis;
		lisaa_listoille(kaikki->tkset, KELLO, nyt.tv_sec);
		SEKTUS = _strlisaa_kopioiden(SEKTUS, sekoitus(tmp));
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
		KIRJOITUSLAJIKSI(avaa_tiedosto);
	      break;
	    case SDLK_s:
	      if(tila != seis)
		break;
	      if(kontrol) {
		sprintf(tmp, "%s/%s", kaikki->uloskansio, kaikki->ulosnimi);
		if(tallenna(kaikki->tkset, tmp))
		  sprintf(TEKSTI, "Tallennettiin \"%s\"", kaikki->ulosnimi);
		else
		  sprintf(TEKSTI, "Ei tallennettu \"%s\"", kaikki->ulosnimi);
		LAITOT.tkstal = 1;
	      } else { //s ilman ctrl:ia, vaihdetaan ulosnimi
	        KIRJOITUSLAJIKSI(ulosnimi);
	      }
	      break;
	    case SDLK_BACKSPACE:
	      if(tila == seis && STRTULOS) {
		SEKTUS = _strpoista1(SEKTUS, -1);
		int ind = _ylaske(_yalkuun(FTULOS))-1;
		poista_listoilta(kaikki->tkset, ind);
		if(kaikki->tkset->strtulos)
		  strcpy(KELLO, kaikki->tkset->strtulos->str);
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
	      LAITOT.kello = 1;
	      break;
	    case SDLK_RETURN:
	    case SDLK_KP_ENTER:
	      SDL_RenderClear(kaikki->rend);
	      LAITOT = kaikki_laitot();
	      if(tila == kirjoitustila) {
		SDL_StopTextInput();
	        tila = seis;
	        nostotoimi = (kaikki->vnta_o->valittu)? tarkastelu : aloita;
		switch((int)kirjoituslaji) {
		case aika:
		  /*tällä voi kysyä SDL-version*/
		  if(!strcmp("SDL -v", KELLO)) {
		    SDL_version v;
		    SDL_VERSION(&v);
		    sprintf(KELLO, "%hhu.%hhu.%hhu", v.major, v.minor, v.patch);
		    LAITOT.kello = 1;
		    break;
		  }
		  /*laitetaan tuloksiin se, mitä kirjoitettiin*/
		  while((apucp = strstr(KELLO, ".")))
		    *apucp = ',';
		  lisaa_listoille(kaikki->tkset, KELLO, time(NULL));
		  TEE_TIEDOT;
		  SEKTUS = _strlisaa_kopioiden(SEKTUS, sekoitus(tmp));
		  MUUTA_TULOS;
		  break;
		case ulosnimi:
		  /*vaihdetaan ulosnimi ja kelloon taas aika*/
		  kaikki->muut_b = _strlisaa_kopioiden(kaikki->muut_b, KELLO);
		  strcpy(TEKSTI, "");
		  _strpoista1(kaikki->muut_b->edel, 1);
		  kaikki->ulosnimi = kaikki->muut_b->str;
		  break;
		case tulosalku:
		  sscanf(KELLO, "%i", &apuind);
		  if(apuind <= _ylaske_taakse(STRTULOS)) {
		    kaikki->tulos_o->rullaus += kaikki->tulos_o->alku - (apuind-1);
		    strcpy(TEKSTI, "");
		  } else {
		    strcpy(TEKSTI, "Hähää, eipäs onnistukaan");
		  }
		  break;
		case avaa_tiedosto:
		  lue_tiedosto(kaikki, KELLO);
		  TEE_TIEDOT;
		  LAITOT = kaikki_laitot();
		  break;
		}
		/*koskee kaikkia kirjoituslajeja*/
		if(kaikki->tkset->ftulos)
		  float_kelloksi(KELLO, kaikki->tkset->ftulos->f);
		break;
	      }
	    case SDLK_END:
	      switch(alue) {
	      default:
	      case tulokset:
		kaikki->tulos_o->rullaus = 0;
		LAITOT.tulos = 1;
		break;
	      case jarjestus1:;
		int mahtuu = kaikki->jarj1_o->sij->h / TTF_FontLineSkip(kaikki->jarj1_o->font);
		kaikki->jarj1_o->rullaus = -(_ylaske(SIJARJ)-1 - mahtuu);
		LAITOT.jarj = 1;
		break;
	      case jarjestus2:
		kaikki->jarj2_o->rullaus = 0;
		LAITOT.jarj = 1;
		break;
	      case sektus:
		kaikki->sektus_o->rullaus = 0;
		LAITOT.sektus = 1;
		break;
	      }
	      break;
	    case SDLK_HOME:
	      switch(alue) {
	      default:
	      case tulokset:
		kaikki->tulos_o->rullaus += kaikki->tulos_o->alku;
		LAITOT.tulos = 1;
		break;
	      case jarjestus1:
		kaikki->jarj1_o->rullaus = 0;
		LAITOT.jarj = 1;
		break;
	      case jarjestus2:
		kaikki->jarj2_o->rullaus += kaikki->jarj2_o->alku;
		LAITOT.jarj = 1;
		break;
	      case sektus:
		kaikki->sektus_o->rullaus += kaikki->sektus_o->alku;
		LAITOT.sektus = 1;
		break;
	      }
	      break;
	    case SDLK_ESCAPE:
	      /*pois kirjoitustilasta muuttamatta mitään*/
	      if(tila == kirjoitustila) {
		SDL_StopTextInput();
		tila = seis;
		nostotoimi = (kaikki->vnta_o->valittu)? tarkastelu : aloita;
		if(kaikki->tkset->strtulos)
		  strcpy(KELLO, kaikki->tkset->strtulos->str);
		TEKSTI[0] = '\0';
		LAITOT.tkstal = 1;
		LAITOT.kello = 1;
	      }
	      break;
	    case SDLK_PLUS:
	    case SDLK_KP_PLUS:
	      if(kontrol) {
		SDL_RenderClear(kaikki->rend);
		skaala *= 1.1;
		SDL_RenderSetScale(kaikki->rend, skaala, skaala);
		LAITOT = kaikki_laitot();
	      } else if(tila != kirjoitustila) {
		int tmpind = _ylaske(_yalkuun(STRTULOS)) - 1;
		muuta_sakko(kaikki->tkset, KELLO, tmpind);
		TEE_TIEDOT;
		LAITOT.kello=1;
		MUUTA_TULOS;
	      }
	      break;
	    case SDLK_MINUS:
	    case SDLK_KP_MINUS:
	      if(kontrol) {
		SDL_RenderClear(kaikki->rend);
		skaala /= 1.1;
		SDL_RenderSetScale(kaikki->rend, skaala, skaala);
		LAITOT = kaikki_laitot();
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
		kaikki->kello_o->vari = kaikki->kvarit[0];
		if(sakko==plus)
		  alku.tv_sec -= 2;
		break;
	      case tarkastelu:
	      TARKASTELU:
		sakko = ei;
		gettimeofday(&alku, NULL);
		dalku = alku.tv_sec + alku.tv_usec/1.0e6;
		nostotoimi = aloita;
		tila = tarkastelee;
		kaikki->kello_o->vari = kaikki->kvarit[1];
		break;
	      case ei_mitaan:
		if(tila != kirjoitustila) //pysäytetty juuri äsken
		  nostotoimi = (kaikki->vnta_o->valittu)? tarkastelu : aloita;
		    break;
	      }
	    case SDLK_LCTRL:
	    case SDLK_RCTRL:
	      kontrol = 0;
	      if(tila == kirjoitustila)
		strcpy(TEKSTI, tekstialue[kirjoituslaji]);
	      LAITOT.tkstal = 1;
	      break;
	    }
	  break;
	case SDL_MOUSEBUTTONDOWN:
	  switch(alue) {
	  case kello:
	    if(tapaht.button.button == SDL_BUTTON_LEFT)
	      if(tila == seis)
		KIRJOITUSLAJIKSI(aika);
	    break;
	  case tietoalue:
	    if(tapaht.button.button == SDL_BUTTON_LEFT ||	\
	       tapaht.button.button == SDL_BUTTON_RIGHT) {
	      char rivi = ((tapaht.button.y - kaikki->tluvut_o->toteutuma->y) / \
			   TTF_FontLineSkip(kaikki->tluvut_o->font));
	      int sarake = ((tapaht.button.x - kaikki->tluvut_o->toteutuma->x) / \
			    (kaikki->tluvut_o->toteutuma->w / 6));
	      sarake -= (sarake/2 + 1); //nyt tämä on 0, 1 tai 2
	      LISATD = _strpoista_kaikki(_yalkuun(LISATD));
	      strlista* sektus = (tapaht.button.button == SDL_BUTTON_LEFT)? NULL : SEKTUS;
	      switch (rivi) {
	      case 0:
	      case 1:
	        LISATD = _strpoista_kaikki(LISATD);
		LISATD = tee_lisatiedot(kaikki->tkset, sektus, avgind[sarake]-4, 5);
		break;
	      case 2:
	      case 3:
	        LISATD = _strpoista_kaikki(LISATD);
		LISATD = tee_lisatiedot(kaikki->tkset, sektus, avgind[sarake+3]-11, 12);
		break;
	      case 4:
	        LISATD = _strpoista_kaikki(LISATD);
		break;
	      }
	      if(tapaht.button.button == SDL_BUTTON_RIGHT) {
		int pit = 0;
		strlista* apu = LISATD;
		while(apu) {
		  pit += strlen(apu->str)+1;
		  apu = apu->seur;
		}
		char tmp_oikea[pit+1];
		_strstulostaf(tmp_oikea, "%s\n", LISATD);
		SDL_SetClipboardText(tmp_oikea);
		LISATD = _strpoista_kaikki(LISATD);
	      } else {
		LAITOT.lisatd=1;
	      }
	    }
	    break;
	  case tulokset:
	    if(tapaht.button.button == SDL_BUTTON_MIDDLE)
	      if(tila == seis)
		KIRJOITUSLAJIKSI(tulosalku);
	    break;
	  case muut:;
	    int rivi = LISTARIVI(muut_o);
	    if(rivi == _ylaske(kaikki->muut_a))
	      rivi--;
	    char* tmpstr = ((strlista*)(_ynouda(kaikki->muut_a, rivi)))->str;
	    if(!strcmp(tmpstr, "ulosnimi:")) {
	      KIRJOITUSLAJIKSI(ulosnimi);
	    } else if(!strcmp(tmpstr, "eri_sekunnit")) {
	      laita_eri_sekunnit(kaikki, tmp);
	    } else if(!strcmp(tmpstr, "kuvaaja")) {
	      FILE *f = fopen(".kuvaaja.bin", "wb");
	      flista* tmpfl = _yalkuun(kaikki->tkset->ftulos);
	      while(tmpfl) {
		fwrite(&(tmpfl->f), 1, sizeof(tmpfl->f), f);
		tmpfl = tmpfl->seur;
	      }
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
	    } else if(!strcmp(tmpstr, "nauhoituslaitteet")) {
	      kaikki->lisatd = _strpoista_kaikki(_yalkuun(kaikki->lisatd));
	      kaikki->lisatd = _yalkuun(nauhoituslaitteet(kaikki->lisatd));
	      LAITOT.lisatd=1;
	    } else if(!strcmp(tmpstr, "ääniajurit")) {
	      kaikki->lisatd = _strpoista_kaikki(_yalkuun(kaikki->lisatd));
	      kaikki->lisatd = _yalkuun(aaniajurit(kaikki->lisatd));
	      LAITOT.lisatd=1;
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
		  system("./kuutio");
		  exit(0);
		}
	      }
	      ipc = liity_muistiin();
	      if(ipc)
		;//liput |= ipc_auki; //tarkista, onko kuutio auki
	    }
	    break;
	  case lisatd:;
	    rivi = LISTARIVI(lisa_o);
	    if(rivi == _ylaske(kaikki->lisatd))
	      rivi--;
	    strlista *apul = _yalkuun(kaikki->lisatd);
	    tmpstr = ((strlista*)(_ynouda(apul, rivi)))->str;
	    if(strstr(((strlista*)(_yloppuun(kaikki->lisatd)))->str, "Ääniajuri nyt: ")) {
	      if(rivi < _ylaske(apul)-1)
		SDL_AudioInit(tmpstr);
	    }
	    if(strstr(((strlista*)(_yloppuun(kaikki->lisatd)))->str, "Nauhoituslaitteita ")) {
	      avaa_aani((rivi==_ylaske(apul)-1)? NULL: tmpstr, TEKSTI);
	      LAITOT.tkstal = 1;
	    }
	    break;
	  default:
	    break;
	  }
	  break;
	case SDL_MOUSEBUTTONUP:
	  if(alue == tarkasteluaikanappi) {
	    if(tapaht.button.button == SDL_BUTTON_LEFT) {
	      kaikki->vnta_o->valittu = (kaikki->vnta_o->valittu+1) % 2;
	      nostotoimi = (kaikki->vnta_o->valittu)? tarkastelu : aloita;
	      LAITOT.valinta=1;
	    }
	  } else if(alue == tulokset) {
	    int tmpind = LISTARIVI(tulos_o);
	    if(tmpind == _ylaske_taakse(kaikki->tkset->strtulos))
	      tmpind--;
	    if(tapaht.button.button == SDL_BUTTON_LEFT) {
	      if(kontrol) {
		/*poistetaan (ctrl + hiiri1)*/
		poista_listoilta(kaikki->tkset, tmpind);
		_strpoista1(_ynouda(_yalkuun(SEKTUS), tmpind), 1);
		TEE_TIEDOT;
		alue = hae_alue(tapaht.button.x, tapaht.button.y, kaikki);
	      } else {
		/*kopioidaan leikepöydälle (hiiri1)*/
	        char* tmpstr = ((strlista*)_ynoudaf(STRTULOS, tmpind, 0))->str;
		time_t aika_t = ((ilista*)_ynoudaf(HETKI, tmpind, 0))->i;
		struct tm *aika = localtime(&aika_t);
		strftime(TEKSTI, 150, "%A %d.%m.%Y klo %H.%M", aika);
		char* tmpsekt = ((strlista*)_ynoudaf(SEKTUS, tmpind, 0))->str;
		sprintf(tmp, "%s; %s\n%s", tmpstr, TEKSTI, tmpsekt);
		SDL_SetClipboardText(tmp);
	      }
	    } else if (tapaht.button.button == SDL_BUTTON_RIGHT) {
	      strlista* tmpstr = _ynouda(_yalkuun(STRTULOS), tmpind);
	      muuta_sakko(kaikki->tkset, (STRTULOS == tmpstr)? KELLO : tmp, tmpind);
	      TEE_TIEDOT;
	    }
	    MUUTA_TULOS;
	    LAITOT.kello=1;
	  }
	  break;
	case SDL_MOUSEWHEEL:
	  if(kontrol) {
	    switch(alue) {
	    case kello:
	      vaihda_fonttikoko(kaikki->kello_o, tapaht.wheel.y*4);
	      LAITOT.kello = 1;
	      break;
	    case tulokset:
	      vaihda_fonttikoko(kaikki->tulos_o, tapaht.wheel.y);
	      LAITOT.tulos = 1;
	      break;
	    case sektus:
	      vaihda_fonttikoko(kaikki->sektus_o, tapaht.wheel.y);
	      LAITOT.sektus = 1;
	      break;
	    case jarjestus1:
	    case jarjestus2:
	      vaihda_fonttikoko(kaikki->jarj1_o, tapaht.wheel.y);
	      kaikki->jarj2_o->font = kaikki->jarj1_o->font;
	      LAITOT.jarj = 1;
	      break;
	    case lisatd:
	      vaihda_fonttikoko(kaikki->lisa_o, tapaht.wheel.y);
	      LAITOT.lisatd = 1;
	      break;
	    default:
	      break;
	    }
	  } else {
	    switch(alue) {
	    case tulokset:
	      if((kaikki->tulos_o->alku == 0 && tapaht.wheel.y > 0) ||	\
		 (kaikki->tulos_o->rullaus == 0 && tapaht.wheel.y < 0))
		break;
	      kaikki->tulos_o->rullaus += tapaht.wheel.y;
	      LAITOT.tulos=1;
	      break;
	    case sektus:
	      if((kaikki->sektus_o->alku == 0 && tapaht.wheel.y > 0) ||	\
		 (kaikki->sektus_o->rullaus == 0 && tapaht.wheel.y < 0))
		break;
	      kaikki->sektus_o->rullaus += tapaht.wheel.y;
	      LAITOT.sektus=1;
	      break;
	    case jarjestus1:; //laitetaan alusta, joten rullaus ≤ 0
	      o = kaikki->jarj1_o;
	      int riveja = o->toteutuma->h / TTF_FontLineSkip(o->font);
	      if((o->alku + riveja == _ylaske(SIJARJ)-1 && tapaht.wheel.y < 0) || \
		 (o->rullaus == 0 && tapaht.wheel.y > 0))
		break;
	      o->rullaus += tapaht.wheel.y;
	      LAITOT.jarj = 1;
	      break;
	    case jarjestus2:;
	      o = kaikki->jarj2_o;
	      if((o->alku == 0 && tapaht.wheel.y > 0) ||	\
		 (o->rullaus == 0 && tapaht.wheel.y < 0))
		break;
	      o->rullaus += tapaht.wheel.y;
	      LAITOT.jarj = 1;
	      break;
	    case lisatd:
	      o = kaikki->lisa_o;
	      riveja = o->toteutuma->h / TTF_FontLineSkip(o->font);
	      if((o->alku + riveja == _ylaske(kaikki->lisatd) && tapaht.wheel.y < 0) || \
		 (o->rullaus == 0 && tapaht.wheel.y > 0))
		break;
	      o->rullaus += tapaht.wheel.y;
	      LAITOT.lisatd = 1;
	      break;
	    case muut:
	      o = kaikki->muut_o;
	      riveja = o->toteutuma->h / TTF_FontLineSkip(o->font);
	      if((o->alku + riveja == _ylaske(_yalkuun(kaikki->muut_a)) && tapaht.wheel.y < 0) || \
		 (o->rullaus == 0 && tapaht.wheel.y > 0))
		break;
	      o->rullaus += tapaht.wheel.y;
	      LAITOT.muut = 1;
	      break;
	    default:
	      break;
	    }
	    break;
	  }
	case SDL_MOUSEMOTION:;
	  alue_e vanha = alue;
	  alue = hae_alue(tapaht.motion.x, tapaht.motion.y, kaikki);
	  switch(alue) {
	  case kello:
	  case lisatd:
	  case sektus:
	    if(hlaji != teksti) {
	      SDL_FreeCursor(kursori);
	      kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	      SDL_SetCursor(kursori);
	      hlaji = teksti;
	    }
	    break;
	  case jarjestus1:
	  case jarjestus2:;
	    tekstiolio_s* o = kaikki->jarj1_o;
	    if(alue == jarjestus2)
	      o = kaikki->jarj2_o;
	    apuind = (o->alku + (tapaht.button.y - o->toteutuma->y) /	\
		      TTF_FontLineSkip(o->font));
	    if(apuind+1 < _ylaske(SIJARJ)) {
	      sscanf(((strlista*)_ynouda(SIJARJ, apuind+1))->str, "%i", &apuind);
	      apuind--;
	      goto LAITA_AIKA_NAKUVIIN;
	    }
	    LAITOT.tkstal = 1;
	    break;
	  case tulokset:;
	    /*laitetaan aika näkyviin*/
	    apuind = LISTARIVI(tulos_o);
	    if(apuind < _ylaske_taakse(kaikki->tkset->tuloshetki)) {
	    LAITA_AIKA_NAKUVIIN:;
	      time_t aika_t = ((ilista*)_ynoudaf(HETKI, apuind, 0))->i;
	      struct tm *aika = localtime(&aika_t);
	      strftime(TEKSTI, 150, "%A %d.%m.%Y klo %H.%M.%S", aika);
	      LAITOT.tkstal = 1;
	    }
	    /*ei break-komentoa*/
	  case muut:
	  case tietoalue:
	  case tarkasteluaikanappi:
	    if(hlaji != kasi) {
	      SDL_FreeCursor(kursori);
	      kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	      SDL_SetCursor(kursori);
	      hlaji = kasi;
	    }
	    break;
	  case tiedot:
	  case muu:
	    if(hlaji != perus) {
	      SDL_FreeCursor(kursori);
	      kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	      SDL_SetCursor(kursori);
	      hlaji = perus;
	    }
	    break;
	  }
	  if( (vanha == tulokset && alue != tulokset) ||		\
	      (vanha == jarjestus1 && alue != jarjestus1) ||		\
	      (vanha == jarjestus2 && alue != jarjestus2) ) { //poistuttiin tuloksista
	    LAITOT.tkstal = 1;
	    if(tila != kirjoitustila)
	      strcpy(TEKSTI, "");
	    else
	      strcpy(TEKSTI, tekstialue[kirjoituslaji]);
	  }
	  
	  break;
	case SDL_TEXTINPUT:
	  strcat(KELLO, tapaht.text.text);
	  LAITOT.kello=1;
	  break;
	case SDL_WINDOWEVENT:
	  switch(tapaht.window.event) {
	  case SDL_WINDOWEVENT_RESIZED:
	    SDL_RenderClear(kaikki->rend);
	    LAITOT = kaikki_laitot();
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
      laita_sekoitus(ipc, kaikki->sekoitukset->str);
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
      LAITOT.kello=1;
    } else if (tila == tarkastelee) {
      gettimeofday(&nyt, NULL);
      dnyt = nyt.tv_sec + nyt.tv_usec/1.0e6;
      char aika = 15 - (char)(dnyt - dalku);
      if(aika > 0) {
	sprintf(KELLO, "%3hhi", aika);
      } else if(aika > -2) {
	sprintf(KELLO, " +2");
	sakko = plus;
	kaikki->kello_o->vari = kaikki->kvarit[2];
      } else {
	sprintf(KELLO, "DNF");
	sakko = dnf;
	kaikki->kello_o->vari = kaikki->kvarit[3];
      }
      LAITOT.kello=1;
    }
    
    piirra(kaikki);
    SDL_Delay(kaikki->viive);
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

alue_e hae_alue(int x, int y, kaikki_s *kaikki) {
  if(piste_alueella(x, y, kaikki->kello_o->toteutuma))
    return kello;
  if(piste_alueella(x, y, kaikki->tulos_o->toteutuma))
    return tulokset;
  if(piste_alueella(x, y, kaikki->jarj1_o->toteutuma))
    return jarjestus1;
  if(piste_alueella(x, y, kaikki->jarj2_o->toteutuma))
    return jarjestus2;
  if(piste_alueella(x, y, kaikki->tiedot_o->toteutuma))
    return tiedot;
  if(piste_alueella(x, y, kaikki->sektus_o->toteutuma))
    return sektus;
  if(piste_alueella(x, y, kaikki->vnta_o->kuvat->sij))
    return tarkasteluaikanappi;
  if(piste_alueella(x, y, kaikki->muut_o->toteutuma))
    return muut;
  if(piste_alueella(x, y, kaikki->lisa_o->toteutuma))
    return lisatd;
  if(piste_alueella(x, y, kaikki->tluvut_o->toteutuma)) {
    if(((x - kaikki->tluvut_o->toteutuma->x) / (kaikki->tluvut_o->toteutuma->w / 6)) % 2)
      return tietoalue;
  }
  return muu;
}

char N = 4;
char* sekoitus(char* s) {
  short pit;
  if(N == 2)
    pit = 9;
  else
    pit = (N-2)*20;
  char paksuus = N/2;
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
    paks = 0;
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
    if(paks) pinta += 6; //esim R --> r jne.

    /*lisätään uudet kiellot*/
    paksKiellot[puolisko][paksKieltoja[puolisko]] = paks;
    if(++paksKieltoja[puolisko] >= paksuus) {
      if(puoliskokielto) {
	akselikielto = 1;
	kieltoakseli = akseli;
      } else {
	puoliskokielto = 1;
	sallittuPuolisko = (puolisko+1) % 2;
      }
    }
    
    /*tulostus*/
    if(N > 5 && paks)
      if(i==0) {
	sprintf(s, "%hhu%c%c", paks+1, pinnat[pinta], suunnat[rand() % 3]);
      } else {
	sprintf(s, "%s %hhu%c%c", s, paks+1, pinnat[pinta], suunnat[rand() % 3]);
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

inline void __attribute__((always_inline)) laita_eri_sekunnit(kaikki_s* k, char* tmps) {
  int *ia = eri_sekunnit(k->tkset->fjarj->seur, NULL, 0);
  int tmp=0;
  int hyv = _ylaske(k->tkset->fjarj->seur);
  int yht = _ylaske(_yalkuun(k->tkset->ftulos));
  int dnf = yht - hyv;
  k->lisatd = _strpoista_kaikki(_yalkuun(k->lisatd));
  k->lisatd = _strlisaa_kopioiden(k->lisatd, "aika  määrä");
  float osuus;
  float kertuma = 0;
  while(ia[tmp] != -1) {
    osuus = ia[tmp+1]/(float)yht;
    kertuma += osuus;
    sprintf(tmps, "%i    %i    %.3f    %.3f",		\
	    ia[tmp], ia[tmp+1], osuus, kertuma);
    k->lisatd = _strlisaa_kopioiden(k->lisatd, tmps);
    tmp+=2;
  }
  if(dnf) {
    osuus = dnf/(float)yht;
    kertuma += osuus;
    sprintf(tmps, "DNF  %i    %.3f    %.3f", dnf, osuus, kertuma);
    k->lisatd = _strlisaa_kopioiden(k->lisatd, tmps);
  }
  free(ia);
  k->lisatd = _yalkuun(k->lisatd);
  k->laitot->lisatd = 1;
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
