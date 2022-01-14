#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include "grafiikka.h"
#include "tulokset.h"
#include "liity_muistiin.h"
#include "asetelma.h"

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

int piste_alueella(int x, int y, SDL_Rect* alue);
alue_e hae_alue(int x, int y);
char* sekoitus(char* s);
void laita_eri_sekunnit(char* tmp);
void vaihda_fonttikoko(tekstiolio_s* olio, int y);
void vaihda_fonttikoko_abs(tekstiolio_s* olio, int y);
void laita_sekoitus(shmRak_s* ipc, char* sek);
void rullaustapahtuma_alusta(tekstiolio_s*, int, SDL_Event);
void rullaustapahtuma_lopusta(tekstiolio_s*, SDL_Event);
void ulosnimeksi(const char*);
void taustaprosessina(const char* restrict);
int viimeinen_sij(char* s, char c);
void avaa_kuutio();

#define KELLO (kellool.teksti)
#define TEKSTI (tkstalol.teksti)
#define HETKI tuloshetki
#define LISTARIVI(nimi, tapahtlaji) ((nimi).alku +			\
				     (tapaht.tapahtlaji.y - (nimi).toteutuma.y) / \
				     TTF_FontLineSkip((nimi).font))
#define TEE_TIEDOT tee_tiedot(avgind);
#define KIRJOITUSLAJIKSI(laji) {			   \
    SDL_StartTextInput();				   \
    SDL_SetTextInputRect(&kellool.sij);			   \
    tila = kirjoitustila;				   \
    kirjoituslaji = laji;				   \
    nostotoimi = ei_mitaan;				   \
    strcpy(KELLO, "");					   \
    strcpy(TEKSTI, tekstialue[kirjoituslaji]);		   \
    laitot = kaikki_laitot;				   \
  }
#define LAITOT (laitot = (tila == seis)? jaaduta : kaikki_laitot)

extern float skaala;
extern char* apuc;

int kaunnista() {
  SDL_Event tapaht;
  struct timeval alku, nyt;
  short min, sek, csek;
  double dalku, dnyt;
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
  int kontrol = 0;
  ipc = NULL;
  nostotoimi = (tarknap.valittu)? tarkastelu : aloita;
  alue_e alue = muual;
  sakko_e sakko;
  SDL_Cursor* kursori;
  kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  SDL_SetCursor(kursori);

  char* apucp;

  TEE_TIEDOT;
  vakiosijainnit();
  LAITOT;
  SDL_StopTextInput();
  slistalle_kopioiden(sektus, sekoitus(apuc));
 TOISTOLAUSE:
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
	      slistalle_kopioiden(sektus, sekoitus(apuc));
	      TEE_TIEDOT;
	      laitot = jaaduta;
	    }
	    break;
	  case SDLK_TAB:
	  JATKA:
	    if(tila == seis) {
	      tila = juoksee;
	      nostotoimi = ei_mitaan;
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
	      if(!(apuind = tallenna(ulosnimi)))
		sprintf(TEKSTI, "Tallennettiin \"%s\"", ulosnimi);
	      else if (apuind < 0)
		sprintf(TEKSTI, "Tallennettiin uusi tiedosto \"%s\"", ulosnimi);
	      else
		sprintf(TEKSTI, "Ei tallennettu \"%s\"", ulosnimi);
	      laitot |= tkstallai;
	    } else { //s ilman ctrl:ia, vaihdetaan ulosnimi
	      KIRJOITUSLAJIKSI(ulosnimiKirj);
	      strncpy(KELLO, ulosnimi, viimeinen_sij(ulosnimi,'/')+1);
	    }
	    break;
	  case SDLK_a:
	    if(tila == seis)
	      KIRJOITUSLAJIKSI(aikaKirj);
	    break;
	  case SDLK_k:
	    if(tila == seis)
	      avaa_kuutio();
	    break;
	  case SDLK_BACKSPACE:
	    if(tila == seis && stulos->pit>0) {
	      poista_listoilta_viimeinen();
	      if(stulos->pit)
		strcpy(KELLO, *VIIMEINEN(stulos));
	      TEE_TIEDOT;
	      laitot = jaaduta;
	    } else if (tila == kirjoitustila) {
	      /*koko utf-8-merkki pois kerralla*/
	      char jatka = 1;
	      while(jatka) {
		jatka = ( (KELLO[strlen(KELLO)-1] & 0xc0) == 0x80 )? 1 : 0; //alkaako 10:lla
		KELLO[strlen(KELLO)-1] = '\0';
	      }
	    }
	    break;
	  case SDLK_RETURN:
	  case SDLK_KP_ENTER:
	    if(tila != kirjoitustila) {
	      LAITOT;
	      break;
	    }
	    laitot = jaaduta;
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
		continue; //jos olisi break, niin tulisi float_kelloksi
	      }
	      /*laitetaan tuloksiin se, mitä kirjoitettiin*/
	      while((apucp = strstr(KELLO, ".")))
		*apucp = ',';
	      float_kelloksi(KELLO, lue_kellosta(KELLO)); //tämä muotoilee syötteen
	      lisaa_listoille(KELLO, time(NULL));
	      TEE_TIEDOT;
	      slistalle_kopioiden(sektus, sekoitus(apuc));
	      KIRJOITUSLAJIKSI(aikaKirj); //usein halutaan syöttää monta peräkkäin
	      KELLO[0] = '\0';
	      continue;
	    case ulosnimiKirj:
	      /*vaihdetaan ulosnimi ja kelloon taas aika*/
	      ulosnimeksi(KELLO);
	      TEKSTI[0] = '\0';
	      break;
	    case tulosalkuKirj:
	      sscanf(KELLO, "%i", &apuind);
	      if(apuind <= stulos->pit) {
		tulosol.rullaus += tulosol.alku - (apuind-1);
		strcpy(TEKSTI, "");
	      } else {
		strcpy(TEKSTI, "Hähää, eipäs onnistukaan");
	      }
	      break;
	    case kuutionKokoKirj:
	      sscanf(KELLO, "%u", &NxN);
	      strcpy(TEKSTI, "");
	      poista_slistalta_viimeinen(sektus);
	      slistalle_kopioiden(sektus, sekoitus(apuc));
	      break;
	    case avaa_tiedostoKirj:
	      lue_tiedosto(KELLO, "");
	      TEE_TIEDOT;
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
	  case SDLK_END:
	    switch(alue) {
	    default:
	    case tuloksetal:
	      tulosol.rullaus = 0;
	      laitot |= tuloslai;
	      break;
	    case jarjestus1al:;
	      int mahtuu = jarjol1.sij.h / TTF_FontLineSkip(jarjol1.font);
	      jarjol1.rullaus = -(stulos->pit - mahtuu) * (mahtuu<stulos->pit);
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
	      if(stulos->pit>0)
		strcpy(KELLO, *VIIMEINEN(stulos));
	      TEKSTI[0] = '\0';
	      laitot = jaaduta;
	    }
	    break;
	  case SDLK_PLUS:
	  case SDLK_KP_PLUS:
	    if(kontrol) {
	      SDL_RenderClear(rend);
	      skaala *= 1.1;
	      SDL_RenderSetScale(rend, skaala, skaala);
	      LAITOT;
	    } else if(tila != kirjoitustila) {
	      muuta_sakko(KELLO, stulos->pit-1);
	      TEE_TIEDOT;
	      LAITOT;
	    }
	    break;
	  case SDLK_MINUS:
	  case SDLK_KP_MINUS:
	    if(kontrol) {
	      SDL_RenderClear(rend);
	      skaala /= 1.1;
	      SDL_RenderSetScale(rend, skaala, skaala);
	      LAITOT;
	    }
	    break;
	  case SDLK_PAUSE:
	    if(kontrol)
	      asm("int $3"); //jäljityspisteansa: debuggerille breakpoint
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
	      laitot = kaikki_laitot;
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
	      laitot = kaikki_laitot;
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
	    char** sektus1 = (tapaht.button.button == SDL_BUTTON_LEFT)? NULL : sektus->taul;
	    switch (rivi) {
	    case 0:
	    case 1:
	      tee_lisatiedot(sektus1, avgind[sarake], 5);
	      break;
	    case 2:
	    case 3:
	      tee_lisatiedot(sektus1, avgind[sarake+3], 12);
	      break;
	    case 4:
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
	      tuhjenna_slista(lisatd);
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
	  if(rivi == muut_a->pit)
	    rivi--;
	  char* tmpstr = muut_a->taul[rivi];
	  if(!strcmp(tmpstr, "ulosnimi:")) {
	    KIRJOITUSLAJIKSI(ulosnimiKirj);
	  } else if(!strcmp(tmpstr, "eri_sekunnit")) {
	    laita_eri_sekunnit(apuc);
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
	    avaa_kuutio();
	  } else if(!strcmp(tmpstr, "autokuutio")) {
	    /*avataan autokuutio taustaprosessina, tämä on pelkkää pelleilyä*/
	    sprintf(apuc, "./kuutio.d/autokuutio %u", NxN);
	    taustaprosessina(apuc);
	    ipc = liity_muistiin();
	    strcpy(TEKSTI, "Aloita välilyönnillä");
	      laitot |= tkstallai;
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
	  if(apuind+1 == ftulos->pit)
	    break;
	  apuind = jarjes[apuind];
	MBUP_TULOKSIA:
	  if(tapaht.button.button == SDL_BUTTON_LEFT) {
	    if(kontrol) {
	      /*poistetaan (ctrl + hiiri1)*/
	      poista_listoilta(apuind);
	      TEE_TIEDOT;
	      alue = hae_alue(tapaht.button.x, tapaht.button.y);
	    } else {
	      /*kopioidaan leikepöydälle (hiiri1)*/
	      char* tmpstr = stulos->taul[apuind];
	      time_t aika_t = thetki->taul[apuind];
	      struct tm *aika = localtime(&aika_t);
	      strftime(TEKSTI, 150, "%A %d.%m.%Y klo %H.%M", aika);
	      /*sekoituksia ei tallenneta, joten tätä ei välttämättä ole*/
	      if(sektus->taul[apuind]) {
		sprintf(apuc, "%s; %s\n%s", tmpstr, TEKSTI, sektus->taul[apuind]);
		SDL_SetClipboardText(apuc);
	      }
	    }
	  } else if (tapaht.button.button == SDL_BUTTON_RIGHT) {
	    muuta_sakko((apuind==stulos->pit-1)? KELLO: apuc, apuind);
	    TEE_TIEDOT;
	  }
	  LAITOT;
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
	  vakiosijainnit();
	} else {
	  switch(alue) {
	  case tuloksetal:
	    rullaustapahtuma_lopusta(&tulosol, tapaht);
	    laitot |= tuloslai;
	    break;
	  case sektusal:
	    rullaustapahtuma_lopusta(&sektusol, tapaht);
	    laitot |= sektuslai;
	    break;
	  case jarjestus1al: //laitetaan alusta, joten rullaus ≤ 0
	    rullaustapahtuma_alusta(&jarjol1, ftulos->pit, tapaht);
	    laitot |= jarjlai;
	    break;
	  case jarjestus2al:
	    rullaustapahtuma_lopusta(&jarjol2, tapaht);
	    laitot |= jarjlai;
	    break;
	  case lisatdal:
	    rullaustapahtuma_alusta(&lisaol, lisatd->pit, tapaht);
	    laitot |= lisatdlai;
	    break;
	  case muutal:
	    rullaustapahtuma_alusta(&muutol, muut_a->pit, tapaht);
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
	  if(apuind < ftulos->pit) {
	    apuind = jarjes[apuind];
	    goto LAITA_AIKA_NAKUVIIN;
	  }
	  laitot |= tkstallai;
	  break;
	case tuloksetal:;
	  /*laitetaan aika näkyviin*/
	  apuind = LISTARIVI(tulosol, motion);
	  if(apuind < thetki->pit) {
	  LAITA_AIKA_NAKUVIIN:;
	    time_t aika_t = thetki->taul[apuind];
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
	break;
      case SDL_WINDOWEVENT:
	switch(tapaht.window.event) {
	case SDL_WINDOWEVENT_RESIZED:
	  SDL_RenderClear(rend);
	  ikkuna_w = tapaht.window.data1;
	  ikkuna_h = tapaht.window.data2;
	  SDL_DestroyTexture(tausta);
	  tausta = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ikkuna_w, ikkuna_h);
	  LAITOT;
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
    laita_sekoitus(ipc, *VIIMEINEN(sektus));
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
  case ipcJatka:
    ipc->viesti = 0;
    goto JATKA;
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
	      (sakko==dnf)? "Ø(" : "", sek, csek/10, csek%10,	\
	      (sakko==ei)? "" : ( (sakko==plus)? "+" : ")" ));
    else
      sprintf(KELLO, "%s%hi:%hi%hi,%hi%hi%s",				\
	      (sakko==dnf)? "Ø(" : "",					\
	      min, sek/10, sek%10, csek/10, csek%10,			\
	      (sakko==ei)? "" : ( (sakko==plus)? "+" : ")" ));
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
  }

  piirra();
  laitot = kellolai * (tila != seis);
  SDL_Delay(viive);
  goto TOISTOLAUSE;
}

int piste_alueella(int x, int y, SDL_Rect* alue) {
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
  short pit = (NxN==2)? 9 : (NxN-2)*20;
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
  int *erisek = eri_sekunnit(ftulos);
  float osuus;
  float kertuma = 0;
  int i;
  tuhjenna_slista(lisatd);
  slistalle_kopioiden(lisatd, "aika  määrä");
  for(i=0; erisek[i] >= 0; i+=2) { //erisek päättyy negatiiviseen ja dnf < -1
    osuus = erisek[i+1]/(float)ftulos->pit; //tmp on sekunti, tmp+1 on näitten määrä
    kertuma += osuus;
    sprintf(tmps, "%i    %i    %.3f    %.3f",		\
	    erisek[i], erisek[i+1], osuus, kertuma);
    slistalle_kopioiden(lisatd, tmps);
  }
  if(erisek[i] < -1) {
    osuus = erisek[i+1]/(float)ftulos->pit;
    kertuma += osuus;
    sprintf(tmps, "DNF  %i    %.3f    %.3f", erisek[i+1], osuus, kertuma);
    slistalle_kopioiden(lisatd, tmps);
  }
  free(erisek);
  laitot |= lisatdlai;
  return;
}

inline void __attribute__((always_inline)) vaihda_fonttikoko_abs(tekstiolio_s* olio, int y) {
  vaihda_fonttikoko(olio, y-olio->fonttikoko);
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

/*rullaus <= 0*/
void rullaustapahtuma_alusta(tekstiolio_s* o, int pit, SDL_Event tapaht) {
  int riveja = o->toteutuma.h / TTF_FontLineSkip(o->font);
  o->rullaus += tapaht.wheel.y;
  o->rullaus *= o->rullaus <= 0; //if(tämä > 0) tämä = 0;
  if(-o->rullaus + riveja > pit)
    o->rullaus = -pit+riveja;
}

/*rullaus >= 0*/
void rullaustapahtuma_lopusta(tekstiolio_s* o, SDL_Event tapaht) {
  if((o->alku <= 0 && tapaht.wheel.y > 0) ||	\
     (o->rullaus <= 0 && tapaht.wheel.y < 0))
    return;
  o->rullaus += tapaht.wheel.y;
  o->rullaus *= o->rullaus >= 0;
  return;
}

void ulosnimeksi(const char* nimi) {
  strcpy(ulosnimi, nimi); 
  laitot |= muutlai;
}

void taustaprosessina(const char* restrict komento) {
  int pid1 = fork();
  if(pid1 > 0)
    waitpid(pid1, NULL, 0);
  else if(!pid1) {
    int pid2 = fork();
    if(pid2 > 0)
      _exit(0);
    else if(!pid2) {
      system(komento);
      exit(0);
    }
  }
}

int viimeinen_sij(char* s, char c) {
  int sij=-1, yrite=-1;
  do
    if(s[++yrite] == c)
      sij = yrite;
  while(s[yrite]);
  return sij;
}

void avaa_kuutio() {
  sprintf(apuc, "./kuutio.d/kuutio %u", NxN);
  taustaprosessina(apuc);
  ipc = liity_muistiin();
  strncpy(apuc, ulosnimi, viimeinen_sij(ulosnimi, '/')+1);
  sprintf(apuc+viimeinen_sij(ulosnimi,'/')+1, "kuutio%i.txt", NxN);
  ulosnimeksi(apuc);
}
