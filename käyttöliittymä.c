#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <flista.h>
#include <strlista.h>
#include <time.h>
#include "rakenteet.h"
#include "grafiikka.h"
#include "tulokset.h"
#include <math.h>
#include <lista_math.h>

typedef enum {
  kello,
  tulokset,
  tiedot,
  sektus,
  tarkasteluaikanappi,
  tietoalue,
  muu
} alue_e;

char piste_alueella(int x, int y, SDL_Rect* alue);
alue_e hae_alue(int x, int y, kaikki_s *kaikki);
char* sekoitus(char* s);

#define SEKTUS (kaikki->sekoitukset)
#define TIEDOT (kaikki->tiedot)
#define KELLO (kaikki->kello_o->teksti)
#define LAITOT (*(kaikki->laitot))
#define STRTULOS (kaikki->tkset->strtulos)
#define FTULOS (kaikki->tkset->ftulos)
#define HETKI (kaikki->tkset->tuloshetki)
#define SJARJ (kaikki->tkset->strjarj)
#define SIJARJ (kaikki->tkset->sijarj)
#define FJARJ (kaikki->tkset->fjarj)
#define LISATD (kaikki->lisatd)
#define MUUTA_TULOS LAITOT.sektus=1; LAITOT.tulos=1; LAITOT.jarj=1; LAITOT.tiedot=1; LAITOT.lisatd=1

int kaunnista(kaikki_s *kaikki) {
  SDL_Event tapaht;
  struct timeval alku, nyt;
  short min, sek, csek;
  double dalku, dnyt;
  char tmp[200];
  int avgind[6];
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
  char kontrol = 0;
  nostotoimi = (kaikki->vnta_o->valittu)? tarkastelu : aloita;
  alue_e alue = muu;
  sakko_e sakko;
  SDL_Cursor* kursori;
  kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  SDL_SetCursor(kursori);

  char* apucp;

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
	      /*pysäytä*/
	      if(tila == juoksee) {
		tila = seis;
		lisaa_listoille(kaikki->tkset, KELLO, nyt.tv_sec);
		SEKTUS = _strlisaa_kopioiden(SEKTUS, sekoitus(tmp));
		TIEDOT = tee_tiedot(TIEDOT, FTULOS, avgind);
		MUUTA_TULOS;
	      }
	      break;
	    case SDLK_LCTRL:
	    case SDLK_RCTRL:
	      kontrol = 1;
	      break;
	    case SDLK_s:
	      if(kontrol)
		tallenna(kaikki->tkset, kaikki->ulosnimi);
	      break;
	    case SDLK_BACKSPACE:
	      if(tila == seis && STRTULOS) {
		SEKTUS = _strpoista1(SEKTUS, -1);
		int ind = _ylaske(_yalkuun(FTULOS))-1;
		poista_listoilta(kaikki->tkset, ind);
		strcpy(KELLO, kaikki->tkset->strtulos->str);
		TIEDOT = tee_tiedot(TIEDOT, FTULOS, avgind);
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
	      if(tila == kirjoitustila) {
		SDL_StopTextInput();
	        tila = seis;
	        nostotoimi = (kaikki->vnta_o->valittu)? tarkastelu : aloita;
		/*laitetaan tuloksiin se, mitä kirjoitettiin*/
		while((apucp = strstr(KELLO, ".")))
		  *apucp = ',';
	        lisaa_listoille(kaikki->tkset, KELLO, time(NULL));
		TIEDOT = tee_tiedot(TIEDOT, FTULOS, avgind);
		SEKTUS = _strlisaa_kopioiden(SEKTUS, sekoitus(tmp));
		LAITOT.kello = 1;
	        MUUTA_TULOS;
	      }
	      break;
	    case SDLK_PLUS:
	    case SDLK_KP_PLUS:
	      if(tila != kirjoitustila) {
		int tmpind = _ylaske(_yalkuun(STRTULOS)) - 1;
		muuta_sakko(kaikki->tkset, KELLO, tmpind);
		TIEDOT = tee_tiedot(TIEDOT, FTULOS, avgind);
		LAITOT.kello=1;
		MUUTA_TULOS;
		break;
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
	      break;
	    }
	  break;
	case SDL_MOUSEBUTTONDOWN: //tässä on vain tietoalue
	  if(alue == tietoalue) {
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
	  }
	  break;
	case SDL_MOUSEBUTTONUP:
	  if(alue == kello && tila != juoksee) {
	    if(tapaht.button.button == SDL_BUTTON_LEFT) {
	      SDL_StartTextInput();
	      tila = kirjoitustila;
	      nostotoimi = ei_mitaan;
	      SDL_SetTextInputRect(kaikki->kello_o->sij);
	      strcpy(KELLO, "");
	      LAITOT.kello=1;
	    }
	  } else if(alue == tarkasteluaikanappi) {
	    if(tapaht.button.button == SDL_BUTTON_LEFT) {
	      kaikki->vnta_o->valittu = (kaikki->vnta_o->valittu+1) % 2;
	      nostotoimi = (kaikki->vnta_o->valittu)? tarkastelu : aloita;
	      LAITOT.valinta=1;
	    }
	  } else if(alue == tulokset) {
	    int tmpind = (kaikki->tulos_o->alku +			\
			  (tapaht.button.y - kaikki->tulos_o->toteutuma->y) / \
			  TTF_FontLineSkip(kaikki->tulos_o->font));
	    if(tapaht.button.button == SDL_BUTTON_LEFT) {
	      poista_listoilta(kaikki->tkset, tmpind);
	      _strpoista1(_ynouda(_yalkuun(SEKTUS), tmpind), 1);
	      TIEDOT = tee_tiedot(TIEDOT, FTULOS, avgind);
	      alue = hae_alue(tapaht.button.x, tapaht.button.y, kaikki);
	    } else if (tapaht.button.button == SDL_BUTTON_RIGHT) {
	      strlista* tmpstr = _ynouda(_yalkuun(STRTULOS), tmpind);
	      muuta_sakko(kaikki->tkset, (STRTULOS == tmpstr)? KELLO : tmp, tmpind);
	      TIEDOT = tee_tiedot(TIEDOT, FTULOS, avgind);
	    }
	    MUUTA_TULOS;
	    LAITOT.kello=1;
	  }
	  break;
	case SDL_MOUSEWHEEL:
	  if(alue == tulokset) {
	    if((kaikki->tulos_o->alku == 0 && tapaht.wheel.y > 0) ||	\
	       (kaikki->tulos_o->rullaus == 0 && tapaht.wheel.y < 0))
	      break;
	    kaikki->tulos_o->rullaus += tapaht.wheel.y;
	    LAITOT.tulos=1;
	  } else if (alue == sektus) {
	    if((kaikki->sektus_o->alku == 0 && tapaht.wheel.y > 0) ||	\
	       (kaikki->sektus_o->rullaus == 0 && tapaht.wheel.y < 0))
	      break;
	    kaikki->sektus_o->rullaus += tapaht.wheel.y;
	    LAITOT.sektus=1;
	  }
	  break;
	case SDL_MOUSEMOTION:
	  alue = hae_alue(tapaht.motion.x, tapaht.motion.y, kaikki);
	  switch(alue) {
	  case kello:
	  case sektus:
	    if(hlaji != teksti) {
	      SDL_FreeCursor(kursori);
	      kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	      SDL_SetCursor(kursori);
	      hlaji = teksti;
	    }
	    break;
	  case tietoalue:
	  case tulokset:
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
  if(piste_alueella(x, y, kaikki->tiedot_o->toteutuma))
    return tiedot;
  if(piste_alueella(x, y, kaikki->sektus_o->toteutuma))
    return sektus;
  if(piste_alueella(x, y, kaikki->vnta_o->kuvat->sij))
    return tarkasteluaikanappi;
  if(piste_alueella(x, y, kaikki->tluvut_o->toteutuma)) {
    if(((x - kaikki->tluvut_o->toteutuma->x) / (kaikki->tluvut_o->toteutuma->w / 6)) % 2)
      return tietoalue;
  }
  return muu;
}

char* sekoitus(char* s) {
  const short pit = 21;
  enum {
    rl = 0,
    ud,
    fb,
    ei
  } kielto = ei;
  unsigned char luku, pinta, viimepinta, suunta;
  char pinnat[] = "RLUDFB";
  char suunnat[] = " '2";

  /*ensimmäinen*/
  luku = rand() % 18;
  pinta = luku % 6;
  suunta = suunnat[luku % 3];
  sprintf(s, "%c%c", pinnat[pinta], suunta);
  viimepinta = pinta;

  /*loput*/
  for (int i=0; i<pit; i++) {
    if (kielto == ei) {
      luku = rand() % 15;
      pinta = luku % 5;
      if (pinta >= viimepinta) pinta++;
      suunta = suunnat[luku % 3];
      sprintf(s, "%s %c%c", s, pinnat[pinta], suunta);
      
      /*haetaan mahdollinen kielto*/
      kielto = (pinta/2 == viimepinta/2)? pinta/2 : ei;
      viimepinta = pinta;
    } else {
      luku = rand() % 12;
      pinta = luku % 4;
      if (pinta >= kielto*2) pinta+=2;
      suunta = suunnat[luku % 3];
      sprintf(s, "%s %c%c", s, pinnat[pinta], suunta);

      kielto = ei;
      viimepinta = pinta;
    }
  }
  return s;
}
