#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#define HAVE_INLINE
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_statistics.h>
#include "grafiikka.h"
#include "tulokset.h"
#include "asetelma.h"
#include "muistin_jako.h"
#include "äänireuna.h"
#include "modkeys.h"

enum alue_e {
    kelloal,
    tuloksetal,
    jarjestus1al,
    jarjestus2al,
    tiedotal,
    sektusal,
    tietoalue,
    lisatdal,
    valikkoal,
    muual
} alue = muual;

enum {
    seis,
    tarkastelee,
    juoksee,
    kirjoitustila
} tila = seis;

enum hiirilaji {
    perus,
    teksti,
    kasi
} hlaji = perus;
SDL_SystemCursor hiiret[] = {
    SDL_SYSTEM_CURSOR_ARROW,
    SDL_SYSTEM_CURSOR_IBEAM,
    SDL_SYSTEM_CURSOR_HAND,
};
SDL_Cursor* kursori;

enum {
    aikaKirj,
    ulosnimiKirj,
    tulosalkuKirj,
    avaa_tiedostoKirj,
    kuutionKokoKirj,
    karsintaKirj,
} kirjoituslaji = aikaKirj;

enum {
    ei_mitaan,
    aloittaminen,
    tarkastelu,
} nostotoimi;

int käynnistä();
void tarkastele();
void aloita_aika();
int lopeta_aika();
int jatka_aikaa();
char* tulos_merkkijonoksi(int ind, char* mjon);
int edellinen_kohta(const char* suote, int* kohta);
int seuraava_kohta(const char* suote, int* kohta);
void pyyhi(char* suote, int kohta);
int piste_alueella(int x, int y, SDL_Rect* alue);
enum alue_e hae_alue(int x, int y);
char* sekoitus();
void laita_eri_sekunnit(char* tmp);
void tee_kuvaaja();
void vaihda_fonttikoko(tekstiolio_s* olio, int y);
void vaihda_fonttikoko_abs(tekstiolio_s* olio, int y);
void vaihda_tarkasteluaikatila();
void laita_sekoitus(shm_tietue* ipc, char* sek);
void rullaustapahtuma_alusta(tekstiolio_s*, int, SDL_Event);
void rullaustapahtuma_lopusta(tekstiolio_s*, SDL_Event);
void ulosnimeksi(const char*);
void korostukseksi(tekstiolio_s* ol, int ind);
void hiireksi(enum hiirilaji);
void taustaprosessina(const char* restrict);
int viimeinen_sij(char* s, char c);
void avaa_kuutio();
void ääni_lue_lopun_unixaika();
void ääni_lue_alun_unixaika();
int äänitila_seuraava();
void avaa_aanireuna();
void sulje_aanireuna();
double hetkinyt();
double* suoran_sovitus(double*);

#define KELLO (kellool.teksti)
#define TEKSTI (tkstalol.teksti)
#define LISTARIVI(nimi, tapahtlaji) ((nimi).alku +			\
				     (tapaht.tapahtlaji.y - (nimi).toteutuma.y) / \
				     TTF_FontLineSkip((nimi).font))
#define TEE_TIEDOT tee_tiedot(avgind);
#define KIRJOITUSLAJIKSI(laji) {			\
	SDL_StartTextInput();				\
	SDL_SetTextInputRect(&kellool.sij);		\
	tila = kirjoitustila;				\
	kirjoituslaji = laji;				\
	nostotoimi = ei_mitaan;				\
	kohdistin = 0;					\
	strcpy(KELLO, "");				\
	strcpy(TEKSTI, tekstialue[kirjoituslaji]);	\
	laitot = kaikki_laitot;				\
    }
#define LAITOT (laitot = (tila==seis)? jäädytä: kaikki_laitot)

extern float skaala;
int kohdistin=-1; //kasvaa vasemmalle ja negatiivinen on piilotettu
sakko_etype sakko;

static int aaniputki0[2] = {-1,-1}, aaniputki1[2] = {-1,-1};
static struct pollfd poll_aani = {-1, POLLIN, POLLIN};

static struct timeval alku, nyt;
static double dalku;
static int avgind[6];
static char apuc[1500];
static unsigned modkey;

int käynnistä() {
    SDL_Event tapaht;
    short min, sek, csek;
    double dnyt;
    int apuind;
    ipc = NULL;
    nostotoimi = (tarkasteluaikatila)? tarkastelu : aloittaminen;
    kursori = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    SDL_SetCursor(kursori);

    TEE_TIEDOT;
    vakiosijainnit();
    LAITOT;
    SDL_StopTextInput();
    slistalle(sektus, sekoitus());
toistolause:
    while(SDL_PollEvent(&tapaht)) {
	switch(tapaht.type) {
	case SDL_QUIT:
	    SDL_FreeCursor(kursori);
	    return 0;
	case SDL_KEYDOWN:
	    switch(tapaht.key.keysym.sym)
	    {
#define _MODKEYS_SWITCH_KEYDOWN
#include "modkeys.h"
	    case SDLK_SPACE:
		if(lopeta_aika())
		    break; //ei lopetettu
		if(äänitila) {
		    uint8_t kirj = aanireuna_valitse_molemmat;
		    write(aaniputki1[1], &kirj, 1);
		}
		break;
	    case SDLK_TAB:
		jatka_aikaa();
		break;
	    case SDLK_o:
		if(tila == seis) {
		    KIRJOITUSLAJIKSI(avaa_tiedostoKirj);
		    int pit = viimeinen_sij(ulosnimi,'/') + 1;
		    strncpy(KELLO, ulosnimi, pit);
		    KELLO[pit] = '\0';
		}
		break;
	    case SDLK_s:
		if(tila != seis)
		    break;
		if(modkey & CTRL) {
		    if(!(apuind = tallenna(ulosnimi)))
			sprintf(TEKSTI, "Tallennettiin \"%s\"", ulosnimi);
		    else if (apuind < 0)
			sprintf(TEKSTI, "Tallennettiin uusi tiedosto \"%s\"", ulosnimi);
		    else
			sprintf(TEKSTI, "Ei tallennettu \"%s\"", ulosnimi);
		    laitot |= tkstallai;
		} else { //s ilman ctrl:ia, vaihdetaan ulosnimi
		    KIRJOITUSLAJIKSI(ulosnimiKirj);
		    int viim = viimeinen_sij(ulosnimi,'/')+1;
		    strncpy(KELLO, ulosnimi, viim);
		    KELLO[viim] = '\0';
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
		    laitot = jäädytä;
		} else if (tila == kirjoitustila)
		    pyyhi(KELLO,kohdistin);
		break;
	    case SDLK_DELETE:
		if(tila == kirjoitustila)
		    if(seuraava_kohta(KELLO, &kohdistin))
			pyyhi(KELLO,kohdistin);
		break;
	    case SDLK_LEFT:
		edellinen_kohta(KELLO, &kohdistin);
		break;
	    case SDLK_RIGHT:
		seuraava_kohta(KELLO, &kohdistin);
		break;
	    case SDLK_RETURN:
	    case SDLK_KP_ENTER:
		if(tila != kirjoitustila) {
		    LAITOT;
		    break;
		}
		laitot = jäädytä;
		SDL_StopTextInput();
		tila = seis;
		nostotoimi = tarkasteluaikatila? tarkastelu : aloittaminen;
		kohdistin = -1;
		switch((int)kirjoituslaji) {
		case aikaKirj:
		    /*laitetaan tuloksiin se, mitä kirjoitettiin*/
		    char* apucp;
		    while((apucp = strstr(KELLO, ".")))
			*apucp = ',';
		    float_kelloksi(KELLO, lue_kellosta(KELLO)); //tämä muotoilee syötteen
		    lisaa_listoille(KELLO, time(NULL));
		    TEE_TIEDOT;
		    slistalle(sektus, sekoitus());
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
		    *TEKSTI = '\0';
		    poista_slistalta_viimeinen(sektus);
		    slistalle(sektus, sekoitus());
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
		if(ftulos && ftulos->pit)
		    float_kelloksi(KELLO, *VIIMEINEN(ftulos));
		break;
	    case SDLK_END:
		if(tila == kirjoitustila) {
		    kohdistin = -(kohdistin<0);
		    break;
		}
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
		if(tila == kirjoitustila) {
		    if(kohdistin >= 0)
			kohdistin = strlen(KELLO);
		    break;
		}
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
		    nostotoimi = tarkasteluaikatila? tarkastelu : aloittaminen;
		    kohdistin = -1;
		    if(stulos->pit>0)
			strcpy(KELLO, *VIIMEINEN(stulos));
		    TEKSTI[0] = '\0';
		    laitot = jäädytä;
		}
		break;
	    case SDLK_PLUS:
	    case SDLK_KP_PLUS:
		if(modkey & CTRL) {
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
		if(modkey & CTRL) {
		    skaala /= 1.1;
		    SDL_RenderSetScale(rend, skaala, skaala);
		    LAITOT;
		}
		break;
	    case SDLK_PAUSE:
		if(modkey & CTRL)
		    asm("int $3"); //jäljityspisteansa
		break;
	    case SDLK_F2:
		if(äänitila == aani_pois_e) {
		    strcpy(TEKSTI, "Ääniputki ei ole auki");
		    laitot |= tkstallai;
		    break;
		}
		uint8_t kirj = aanireuna_valinta;
		write(aaniputki1[1], &kirj, 1);
		break;
	    }
	    switch(tapaht.key.keysym.scancode) {
	    case SDL_SCANCODE_H:
		if(modkey & ALT)
		    edellinen_kohta(KELLO, &kohdistin);
		break;
	    case SDL_SCANCODE_L:
		if(modkey & ALT)
		    seuraava_kohta(KELLO, &kohdistin);
		break;
	    default:
		break;
	    }
	    break;
	case SDL_KEYUP:
	    switch(tapaht.key.keysym.sym)
	    {
#define _MODKEYS_SWITCH_KEYUP
#include "modkeys.h"
	    case SDLK_SPACE:
		switch(nostotoimi) {
		case aloittaminen:
		    aloita_aika();
		    break;
		case tarkastelu:
		    tarkastele();
		    break;
		case ei_mitaan:
		    if(tila != kirjoitustila) //pysäytetty juuri äsken
			nostotoimi = tarkasteluaikatila? tarkastelu : aloittaminen;
			    break;
		}
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
		    char rivi = ((tapaht.button.y - tilastoluvutol.toteutuma.y) / \
				 TTF_FontLineSkip(tilastoluvutol.font));
		    int sarake = ((tapaht.button.x - tilastoluvutol.toteutuma.x) / \
				  (tilastoluvutol.toteutuma.w / 6));
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
	    case valikkoal:;
		int rivi = LISTARIVI(valikkool, button);
		switch(rivi) {
		case tarkasteluaika_e:
		    vaihda_tarkasteluaikatila();    break;
		case ulosnimi_e:
		    KIRJOITUSLAJIKSI(ulosnimiKirj); break;
		case eri_sekunnit_e:
		    laita_eri_sekunnit(apuc);       break;
		case kuvaaja_e:
		    tee_kuvaaja();                  break;
		case kuutio_e:
		    avaa_kuutio();                  break;
		case autokuutio_e:
		    /*avataan autokuutio taustaprosessina, tämä on pelkkää pelleilyä*/
		    sprintf(apuc, "./kuutio.d/autokuutio %u", NxN);
		    taustaprosessina(apuc);
		    ipc = liity_muistiin();
		    strcpy(TEKSTI, "Aloita välilyönnillä");
		    laitot |= tkstallai;
		    break;
		case aani_e:
		    äänitila_seuraava();           break;
		}
		break; //case valikkoal
	    default:
		break;
	    }
	    break;
	case SDL_MOUSEBUTTONUP:
	    switch(alue) {
		/*tulos- ja järjestysalue johtavat samaan toimintoon
		  järjestysalueilta luettu indeksi muunnetaan tulosalueen indeksiksi*/
	    case tuloksetal:
		apuind = LISTARIVI(tulosol, button);
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
		    if(modkey & CTRL) {
			/*poistetaan (ctrl + hiiri1)*/
			poista_listoilta(apuind);
			TEE_TIEDOT;
			alue = hae_alue(tapaht.button.x, tapaht.button.y);
		    } else
			SDL_SetClipboardText(tulos_merkkijonoksi(apuind, apuc));
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
	    if(modkey & CTRL) {
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
		case valikkoal:
		    rullaustapahtuma_alusta(&valikkool, muut_a->pit, tapaht);
		    laitot |= valikkolai;
		    break;
		default:
		    break;
		}
		break;
	    }
	case SDL_MOUSEMOTION:;
	    enum alue_e vanha = alue;
	    alue = hae_alue(tapaht.motion.x, tapaht.motion.y);
	    if(vanha != alue) {
		if(korostusol.paksuus > 0)
		    korostusol.paksuus *= -1;
		laitot |= tkstallai;
		if(tila != kirjoitustila)
		    strcpy(TEKSTI, "");
		else
		    strcpy(TEKSTI, tekstialue[kirjoituslaji]);
	    }
	    switch(alue) {
	    case kelloal:
	    case lisatdal:
	    case sektusal:
		hiireksi(teksti);
		break;
	    case jarjestus1al:
		apuind = LISTARIVI(jarjol1, motion);
		korostukseksi(&jarjol1, apuind);
		apuind = jarjes[apuind]; //saman ajan indeksi tuloslistassa
		goto laita_aika_näkyviin;
	    case jarjestus2al:;
		apuind = LISTARIVI(jarjol2, motion);
		korostukseksi(&jarjol2, apuind);
		apuind = jarjes[apuind];
		goto laita_aika_näkyviin;
	    case tuloksetal:;
		apuind = LISTARIVI(tulosol, motion);
		korostukseksi(&tulosol, apuind);
	    laita_aika_näkyviin:;
		time_t aika_t = thetki->taul[apuind];
		struct tm *aika = localtime(&aika_t);
		strftime(TEKSTI, 150, "%A %d.%m.%Y klo %H.%M.%S", aika);
		laitot |= tkstallai;
		hiireksi(kasi);
		break;
	    case valikkoal:
		korostukseksi(&valikkool, LISTARIVI(valikkool, motion));
	    case tietoalue:
		hiireksi(kasi);
		break;
	    case tiedotal:
	    case muual:
		hiireksi(perus);
		break;
	    } //endswitch alue
	    break; //case mousemotion
	case SDL_TEXTINPUT:
	    if(modkey & (ALT|WIN|CTRL))
		break;
	    int pit = strlen(KELLO);
	    char* loppuosa_ptr = KELLO+pit-kohdistin;
	    strcpy(apuc, loppuosa_ptr);
	    strcpy(loppuosa_ptr, tapaht.text.text);
	    strcat(KELLO, apuc);
	    break;
	case SDL_WINDOWEVENT:
	    switch(tapaht.window.event) {
	    case SDL_WINDOWEVENT_RESIZED:
		ikkuna_w = tapaht.window.data1;
		ikkuna_h = tapaht.window.data2;
		SDL_DestroyTexture(tausta);
		tausta = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ikkuna_w, ikkuna_h);
		vakiosijainnit();
		LAITOT;
		break;
	    }
	    break;
	} //switch(tapaht.type)
    } //while(SDL_PollEvent(&tapaht))

    /*SDL-tapahtumat päättyivät
      seuraavaksi tarkistetaan mahdollisen kuution tapahtumat*/
    do {
	if(!ipc || !(ipc->viesti))
	    break;
	switch(ipc->viesti) {
	case ipcAnna_sekoitus:
	    laita_sekoitus(ipc, *VIIMEINEN(sektus));
	    break;
	case ipcTarkastelu:
	    ipc->viesti = 0;
	    tarkastele();
	    break;
	case ipcAloita:
	    ipc->viesti = 0;
	    aloita_aika();
	    break;
	case ipcLopeta:
	    ipc->viesti = 0;
	    lopeta_aika();
	    break;
	case ipcJatka:
	    ipc->viesti = 0;
	    jatka_aikaa();
	    break;
	}
    } while(0);
  
    /*Äänikuuntelijan tapahtumat*/
    while((apuind = poll(&poll_aani, 1, 0))) {
	if(apuind < 0) {
	    fprintf(stderr, "Virhe poll-funktiossa: %s\n", strerror(errno));
	    break;
	}
	if(poll_aani.revents & POLLIN) {
	    uint32_t luenta;
	    if((apuind = read(aaniputki0[0], &luenta, 4)) <= 0) {
		if(apuind < 0)
		    fprintf(stderr, "Virhe äänikuuntelijasta lukemisessa %s\n", strerror(errno));
		sulje_aanireuna(aaniputki0, aaniputki1, &poll_aani);
		break;
	    }
	    switch(luenta) {
	    case seuraavaksi_lopun_unixaika:
		ääni_lue_lopun_unixaika();
		break;
	    case seuraavaksi_alun_unixaika:
		ääni_lue_alun_unixaika();
		break;
	    case havaittiin_reuna:
		if(äänitila == ääni_pysäytys_e) lopeta_aika();
		break;
	    }
	}
	else if(poll_aani.revents & POLLHUP) {
	    while(äänitila_seuraava() != aani_pois_e);
	    strcpy(TEKSTI, "Ääniputki sulkeutui");
	    laitot |= tkstallai;
	}
	else if(poll_aani.revents & POLLERR) {
	    fprintf(stderr, "Virhetila äänikuuntelijassa (POLLERR)\n");
	    while(äänitila_seuraava() != aani_pois_e);
	    strcpy(TEKSTI, "Virhe ääniputkessa");
	    laitot |= tkstallai;
	}
    }

    if(tila == juoksee) {
	gettimeofday(&nyt, NULL);
	sek = (int)((nyt.tv_sec + nyt.tv_usec/1.0e6) - (alku.tv_sec + alku.tv_usec/1.0e6));
	min = sek / 60;
	sek %= 60;
	csek = (nyt.tv_usec - alku.tv_usec + 1000000)/10000 % 100;
	if(!min)
	    sprintf(KELLO, "%s%hi,%hi%hi%s",
		    (sakko==dnf)? "Ø(" : "", sek, csek/10, csek%10,
		    (sakko==ei)? "" : ( (sakko==plus)? "+" : ")" ));
	else
	    sprintf(KELLO, "%s%hi:%hi%hi,%hi%hi%s",
		    (sakko==dnf)? "Ø(" : "",
		    min, sek/10, sek%10, csek/10, csek%10,
		    (sakko==ei)? "" : ( (sakko==plus)? "+" : ")" ));
    }
    else if (tila == tarkastelee) {
	gettimeofday(&nyt, NULL);
	dnyt = nyt.tv_sec + nyt.tv_usec/1.0e6;
	char aika = 15 - (char)(dnyt - dalku);
	if(aika > 0) {
	    sprintf(KELLO, "%3hhi", aika);
	}
	else if(aika > -2) {
	    sprintf(KELLO, " +2");
	    sakko = plus;
	    kellool.vari = kellovärit[2];
	}
	else {
	    sprintf(KELLO, "DNF");
	    sakko = dnf;
	    kellool.vari = kellovärit[3];
	}
    }

    piirrä(1);
    laitot = kellolai * (tila != seis);
    SDL_Delay(viive);
    goto toistolause;
}

double hetkinyt() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + t.tv_usec*1e-6;
}

void tarkastele() {
    sakko = ei;
    dalku = hetkinyt();
    nostotoimi = aloittaminen;
    tila = tarkastelee;
    kellool.vari = kellovärit[1];
    strcpy(TEKSTI, "");
    laitot = kaikki_laitot;
}

void aloita_aika() {
    if(tila != tarkastelee)
	sakko = ei;
    if(äänitila) {
	uint8_t kirj = äänireuna_tallenna_tästä;
	write(aaniputki1[1], &kirj, 1);
    }

    gettimeofday(&alku, NULL);
    nostotoimi = ei_mitaan;
    tila = juoksee;
    kellool.vari = kellovärit[0];
    if(sakko==plus)
	alku.tv_sec -= 2;
    strcpy(TEKSTI, "");
    laitot = kaikki_laitot;
}

int lopeta_aika() {
    if(tila != juoksee)
	return 1;
    tila = seis;
    lisaa_listoille(KELLO, nyt.tv_sec);
    slistalle(sektus, sekoitus());
    TEE_TIEDOT;
    laitot = jäädytä;
    return 0;
}

int jatka_aikaa() {
    if(tila != seis)
	return 1;
    tila = juoksee;
    nostotoimi = ei_mitaan;
    return 0;
}

char* tulos_merkkijonoksi(int apuind, char* mjon) {
    char* tmpstr = stulos->taul[apuind];
    time_t aika_t = thetki->taul[apuind];
    struct tm *aika = localtime(&aika_t);
    size_t pit = strftime(mjon, 256, "%A %d.%m.%Y klo %H.%M", aika);
    if(!pit) {
	strcpy(TEKSTI, "Liian laaja strftime");
	mjon[0] = '\0';
    }
    pit++;
    /*sekoituksia ei tallenneta, joten tätä ei välttämättä ole*/
    if(sektus->pit>apuind && sektus->taul[apuind])
	sprintf(mjon+pit, "%s; %s\n%s", tmpstr, mjon, sektus->taul[apuind]);
    else
	sprintf(mjon+pit, "%s; %s", tmpstr, mjon);
    return mjon+pit;
}

/*näissä siirrytään eteen- tai taakespäin koko utf-8-merkin verran*/
int edellinen_kohta(const char* restrict suote, int* kohta) {
    if(kohta < 0)
	return 0;
    int jatka = 1;
    int pit = strlen(suote);
    int r = 0;
    while(jatka && pit > *kohta) {
	r=1;
	jatka = (suote[pit- ++(*kohta)] & 0xc0) == 0x80;
    }
    return r;
}

int seuraava_kohta(const char* restrict suote, int* kohta) {
    int pit = strlen(suote);
    int r = 0;
    while(*kohta && (r=1) && ((suote[pit- --(*kohta)] & 0xc0) == 0x80));
    return r;
}

void pyyhi(char* suote, int kohta) {
    int pit = strlen(suote);
    char tmpc[pit+1];
    strcpy(tmpc, suote+pit-kohta);
    edellinen_kohta(suote, &kohta);
    strcpy(suote+pit-kohta, tmpc);
}

int piste_alueella(int x, int y, SDL_Rect* alue) {
    if (x < alue->x)
	return 0;
    if (x > alue->x + alue->w)
	return 0;
    if (y < alue->y)
	return 0;
    if (y >= alue->y + alue->h)
	return 0;
    return 1;
}

enum alue_e hae_alue(int x, int y) {
    if(piste_alueella(x, y, &kellool.toteutuma))
	return kelloal;
    if(piste_alueella(x, y, &tulosol.toteutuma))
	return tuloksetal;
    if(piste_alueella(x, y, &jarjol1.toteutuma))
	return jarjestus1al;
    if(piste_alueella(x, y, &jarjol2.toteutuma))
	return jarjestus2al;
    if(piste_alueella(x, y, &tilastotol.toteutuma))
	return tiedotal;
    if(piste_alueella(x, y, &sektusol.toteutuma))
	return sektusal;
    if(piste_alueella(x, y, &valikkool.toteutuma))
	return valikkoal;
    if(piste_alueella(x, y, &lisaol.toteutuma))
	return lisatdal;
    if(piste_alueella(x, y, &tilastoluvutol.toteutuma)) {
	if(((x - tilastoluvutol.toteutuma.x) / (tilastoluvutol.toteutuma.w / 6)) % 2)
	    return tietoalue;
    }
    return muual;
}

int logi_10(int n) {
    int r = 0;
    while(n /= 10) r++;
    return r;
}

char* sekoitus() {
    static const char pinnat[] = "RLUDFBrludfb";
    static const char suunnat[] = " '2";
    unsigned paksuus = NxN/2;
    char akseli, viimeakseli=10, paks;
    int pinta, puolisko;
    char akselikielto=0, kieltoakseli=0, puoliskokielto=0, sallittu_puolisko=0;
    int pakskieltoja[2] = {0};
    int pit = (NxN==2)? 9 : (NxN-2)*20;
    char pakskiellot[2][paksuus];
    int maxpit;

    if(paksuus < 10)
	maxpit = pit * (3+(NxN>5)); // esim. "R2 " (alle 6-kuutio) tai "2R' " (väh 6-kuutio)
    else
	maxpit = pit * (4+logi_10(paksuus)); // logi_10: montako numeroa paksuus voi vaatia, esim. "12R' "
    char* s = malloc(maxpit + 1);
    if(!s) {
	sprintf(TEKSTI, "Sekoitukselle ei ole muistia: %s", strerror(errno));
	laitot |= tkstallai;
	return NULL;
    }
    *s = '\0';

    for (int i=0; i<pit; i++) {
	/*akseli*/
	akseli = rand() % (3-akselikielto); // akselikielto on 1 tai 0
	if (akselikielto && akseli >= kieltoakseli) akseli++;
	if(akseli != viimeakseli) {
	    puoliskokielto = 0;
	    akselikielto = 0;
	    for(int i1=0; i1<2; i1++)
		pakskieltoja[i1] = 0;
	}
	viimeakseli = akseli;
    
	/*puolisko*/
	puolisko = (puoliskokielto)? sallittu_puolisko : rand() % 2;
	pinta = akseli*2 + puolisko;
    
	/*paksuus*/
	// esim N=3 --> % N/2=1 --> aina 0
	char paksind = rand() % (paksuus - pakskieltoja[puolisko]);
	/*haetaan oikea paksuus
	  paksind on indeksi sallittujen paksuuksien joukossa*/
	char loutuneet = -1;
	paks = 1;
	while(1) {
	    char loutui = 1;
	    for(int j=0; j<pakskieltoja[puolisko]; j++)
		if(pakskiellot[puolisko][j] == paks) {
		    loutui = 0;
		    break;
		}
	    if(loutui) // kys paksuus oli sallittu
		if(++loutuneet == paksind)
		    break;
	    paks++;
	}
	if(paks > 1) pinta += 6; // esim R --> r jne.

	/*lisätään uudet kiellot*/
	pakskiellot[puolisko][pakskieltoja[puolisko]] = paks;
	pakskieltoja[puolisko]++;
	/*Parillisilla kuutioilla ei sallita esim N/2u ja N/2d peräkkäin*/
	if(NxN % 2 == 0 && paks == paksuus) {
	    int toinen = !puolisko;
	    pakskiellot[toinen][pakskieltoja[toinen]] = paksuus;
	    pakskieltoja[toinen]++;
	}
	for(int j=0; j<2; j++)
	    if(pakskieltoja[j] >= paksuus) {
		if(puoliskokielto) {
		    akselikielto = 1;
		    kieltoakseli = akseli;
		} else {
		    puoliskokielto = 1;
		    sallittu_puolisko = !j;
		}
	    }
	/*tulostus*/
	if(NxN > 5 && paks)
	    sprintf(s+strlen(s), "%u%c%c ", paks, pinnat[pinta], suunnat[rand() % 3]);
	else
	    sprintf(s+strlen(s), "%c%c ", pinnat[pinta], suunnat[rand() % 3]);
    }
    s[strlen(s)-1] = '\0';
    return s;
}

void vaihda_tarkasteluaikatila() {
    int pit0 = strlen(tarkastelu_str[tarkasteluaikatila]);
    tarkasteluaikatila = !tarkasteluaikatila;
    char* apuc = muut_a->taul[tarkasteluaika_e];
    strcpy(apuc+strlen(apuc)-pit0, tarkastelu_str[tarkasteluaikatila]);
    nostotoimi = (tarkasteluaikatila)? tarkastelu : aloittaminen;
    LAITOT;
}

void laita_eri_sekunnit(char* tmps) {
    int *erisek = eri_sekunnit(ftulos);
    float osuus;
    float kertuma = 0;
    int i;
    tuhjenna_slista(lisatd);
    slistalle_kopioiden(lisatd, "aika  määrä");
    for(i=0; erisek[i] >= 0; i+=2) { // erisek päättyy negatiiviseen ja dnf < -1
	osuus = erisek[i+1]/(float)ftulos->pit; // tmp on sekunti, tmp+1 on näitten määrä
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

double* suoran_sovitus(double* p) {
    double *dtulos = malloc(ftulos->pit * 2 * sizeof(double));
    double *dx = dtulos + ftulos->pit;
    double _;
    if(!dtulos)
	return NULL;
    int pit = 0;
    for(int i=0; i<ftulos->pit; i++)
	if(ftulos->taul[i] < INFINITY) {
	    dtulos[pit  ] = (double)ftulos->taul[i];
	    dx    [pit++] = i;
	}

    gsl_fit_linear(dx, 1, dtulos, 1, pit, p+0, p+1, &_, &_, &_, &_);

    /* Trendin merkitsevyys t-testillä.
       Testisuureen kaava löytyy esim Wikipediasta: Student's t-test.
       Tässä residuaalien pitäisi jakautua normaalisti, mikä tuskin toteutuu. */
    double r = gsl_stats_correlation(dx, 1, dtulos, 1, pit);
    double t = r*sqrt(pit-2.0) / sqrt(1-r*r);
    p[2] = gsl_cdf_tdist_P(t, (double)pit-2);
  
    free(dtulos);
    return p;
}

void tee_kuvaaja() {
    int putki[2];
    char apuc[100];
    pipe(putki);
    double param[3];
    suoran_sovitus(param);
    sprintf(apuc, KOTIKANSIO"/kuvaaja.py %i %i", putki[0], putki[1]);
    taustaprosessina(apuc);
    close(putki[0]);
    write(putki[1], &(ftulos->pit), 4);
    write(putki[1], param, 3*8);
    write(putki[1], ftulos->taul, ftulos->koko*ftulos->pit);
    close(putki[1]);
}

void vaihda_fonttikoko_abs(tekstiolio_s* olio, int y) {
    vaihda_fonttikoko(olio, y-olio->fonttikoko);
}

void vaihda_fonttikoko(tekstiolio_s* olio, int y) {
    TTF_CloseFont(olio->font);
    olio->font = NULL;
    olio->fonttikoko += y;
    olio->font = TTF_OpenFont(olio->fonttied, olio->fonttikoko);
    if(!olio->font)
	fprintf(stderr, "Virhe: Ei avattu fonttia uudesti: %s\n", TTF_GetError());
    return;
}

void laita_sekoitus(shm_tietue* ipc, char* sek) {
    strncpy(ipc->data, sek, SHM_KOKO_DATA-1);
    ipc->viesti = 0;
}

/*rullaus <= 0*/
void rullaustapahtuma_alusta(tekstiolio_s* o, int pit, SDL_Event tapaht) {
    int riveja = o->toteutuma.h / TTF_FontLineSkip(o->font);
    o->rullaus += tapaht.wheel.y;
    o->rullaus *= o->rullaus <= 0; // if(tämä > 0) tämä = 0;
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
    laitot |= valikkolai;
}

void korostukseksi(tekstiolio_s* ol, int ind) {
    korostusol.kulmio.x = ol->toteutuma.x;
    korostusol.kulmio.y = ol->toteutuma.y + (ind-ol->alku)*TTF_FontLineSkip(ol->font);
    korostusol.kulmio.w = ol->toteutuma.w;
    korostusol.kulmio.h = TTF_FontLineSkip(ol->font);
    if(korostusol.paksuus < 0)
	korostusol.paksuus *= -1;
}

void hiireksi(enum hiirilaji laji) {
    if(hlaji == laji)
	return;
    SDL_FreeCursor(kursori);
    kursori = SDL_CreateSystemCursor(hiiret[laji]);
    SDL_SetCursor(kursori);
    hlaji = laji;
}

void sigchld(int turha) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void taustaprosessina(const char* restrict komento) {
    char apuc[100];
    const char* s = komento;
    int argc=0, n;
    while( sscanf(s, "%99s%n", apuc, &n)==1 ) { //lasketaan argumentit
	s += n;
	argc++;
    }
    char* argv[argc+1];
    s = komento;
    for(int i=0; sscanf(s, "%99s%n", apuc, &n)==1; i++) { //argumentit char**-olioon
	argv[i] = strdup(apuc);
	s += n;
    }
    argv[argc] = NULL;
    if( !fork() ) {
	execvp(argv[0], argv); //tämän ei pitäisi palata, huom. että myös 0. argumentti täytyy antaa tässä
	fprintf(stderr, "Virhe taustaprosessissa: %s\n", strerror(errno));
	exit(1);
    }
    for(int i=0; i<argc; i++)
	free(argv[i]);
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
    char apuc[200];
    sprintf(apuc, "skello_kuutio %u", NxN);
    taustaprosessina(apuc);
    ipc = liity_muistiin();
    strncpy(apuc, ulosnimi, viimeinen_sij(ulosnimi, '/')+1);
    sprintf(apuc+viimeinen_sij(ulosnimi,'/')+1, "kuutio%i.txt", NxN);
    ulosnimeksi(apuc);
}

void ääni_lue_lopun_unixaika() {
    int apuind = poll(&poll_aani, 1, 500);
    if(apuind <= 0) {
	if(apuind<0) perror("\033[31mVirhe (ääni_lue_lopun_unixaika)\033[0m");
	else fprintf(stderr, "\033[31mVirhe (ääni_lue_lopun_unixaika)\033[0m: aikaa ei ollut saatavilla\n");
	return;
    }
    uint64_t luenta;
    if(poll_aani.revents & POLLIN) {
	if((apuind = read(aaniputki0[0], &luenta, 8)) == 8) {
	    float tulos = (double)luenta/1000 - ((double)alku.tv_sec+alku.tv_usec/1.0e6);
	    float_kelloksi(KELLO, tulos);
	    *VIIMEINEN(ftulos) = tulos;
	    free(*VIIMEINEN(stulos));
	    *VIIMEINEN(stulos) = strdup(KELLO);
	    TEE_TIEDOT;
	    laitot = jäädytä;
	}
	else if(apuind < 0) perror("\033[31mVirhe 2 (ääni_lue_lopun_unixaika)\033[0m");
	else {
	    sprintf(TEKSTI, "Luettiin %i eikä 8 tavua.", apuind);
	    laitot |= tkstallai;
	}
    }
    if(poll_aani.revents & (POLLHUP|POLLERR|POLLNVAL))
	while(äänitila_seuraava() != aani_pois_e);
}

void ääni_lue_alun_unixaika() {
    int apuind = poll(&poll_aani, 1, 500);
    if(apuind <= 0) {
	if(apuind<0) perror("\033[31mVirhe (ääni_lue_alun_unixaika)\033[0m");
	else fprintf(stderr, "\033[31mVirhe (ääni_lue_alun_unixaika)\033[0m: aikaa ei ollut saatavilla\n");
	return;
    }
    uint64_t luenta;
    if(poll_aani.revents & POLLIN) {
	if((apuind = read(aaniputki0[0], &luenta, 8)) == 8) {
	    alku = (struct timeval){.tv_sec=luenta/1000, .tv_usec=luenta%1000*1000};
	    float tulos = (double)nyt.tv_sec+nyt.tv_usec/1.0e6 - (double)luenta/1000;
	    float_kelloksi(KELLO, tulos);
	    *VIIMEINEN(ftulos) = tulos;
	    free(*VIIMEINEN(stulos));
	    *VIIMEINEN(stulos) = strdup(KELLO);
	    TEE_TIEDOT;
	    laitot = jäädytä;
	}
	else if(apuind < 0) perror("\033[31mVirhe 2 (ääni_lue_alun_unixaika)\033[0m");
	else {
	    sprintf(TEKSTI, "Luettiin %i eikä 8 tavua.", apuind);
	    laitot |= tkstallai;
	}
    }
    if(poll_aani.revents & (POLLHUP|POLLERR|POLLNVAL))
	while(äänitila_seuraava() != aani_pois_e);
}

int äänitila_seuraava() {
    if(äänitila == aani_pois_e)
	avaa_aanireuna(aaniputki0, aaniputki1, &poll_aani);
    äänitila = (äänitila+1) % ääni_vaihtoehtoja;
    if(äänitila == aani_pois_e)
	sulje_aanireuna(aaniputki0, aaniputki1, &poll_aani);
    strcpy(äänitila_str, aanivaihtoehdot[äänitila]);
    laitot |= valikkolai;
    return äänitila;
}

void avaa_aanireuna(int *putki0, int *putki1, struct pollfd* poll_aani) {
    char apuc[200];
    pipe(putki0); pipe(putki1);
    poll_aani->fd = putki0[0];
    sprintf(apuc, "äänireuna --putki1 %i %i --putki0 %i %i", putki0[0], putki0[1], putki1[0], putki1[1]);
    taustaprosessina(apuc);
    close(putki0[1]); close(putki1[0]);
}

void sulje_aanireuna(int* aaniputki0, int* aaniputki1, struct pollfd* poll_aani) {
    if(poll_aani->fd < 0)
	return;
    poll_aani->fd = -1;
    if(close(aaniputki0[0]) < 0)
	fprintf(stderr, "Virhe ääniputki0:n sulkemisessa: %s\n", strerror(errno));
    if(close(aaniputki1[1]) < 0)
	fprintf(stderr, "Virhe ääniputki1:n sulkemisessa: %s\n", strerror(errno));
    aaniputki0[0] = -1;
    aaniputki1[1] = -1;
}

int main(int argc, char** argv) {
    int r = 0;
    signal(SIGCHLD, sigchld);
    setlocale(LC_ALL, getenv("LANG"));
    if (SDL_Init(SDL_INIT_VIDEO)) {
	fprintf(stderr, "Virhe: Ei voi alustaa SDL-grafiikkaa: %s\n", SDL_GetError());
	r = 1;
	goto EI_SDL;
    }
    if (TTF_Init()) {
	fprintf(stderr, "Virhe: Ei voi alustaa SDL_ttf-fonttikirjastoa: %s\n", TTF_GetError());
	SDL_Quit();
	r = 1;
	goto EI_TTF;
    }
    ikkuna = SDL_CreateWindow\
	(ohjelman_nimi, ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h, SDL_WINDOW_RESIZABLE);
    rend = SDL_CreateRenderer(ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);
    SDL_GetWindowSize(ikkuna, &ikkuna_w, &ikkuna_h); // ikkunointimanageri voi muuttaa kokoa pyydetystä
    tausta = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ikkuna_w, ikkuna_h);

    if(asetelma())
	goto EI_FONTTI;
  
    time_t t;
    srand((unsigned) time(&t));
    strcpy(kellool.teksti, " ");

    /** luetaan tiedosto tarvittaessa
	luettavaa voidaan rajata:
	viimeiset 1000 --> -1000:
	alkaen 1000:sta --> 1000:
	1000 ensimmäistä --> :1000
	alkaen 1000:sta viimeiseen 1000:en asti --> 1000:-1000 jne */
    if(argc > 1) {
	int pit = viimeinen_sij(ulosnimi,'/') + 1;
	strncpy(KELLO,ulosnimi,pit);
	strcpy(KELLO+pit,argv[1]);
	if(argc > 2) {
	    if(lue_tiedosto(argv[1], argv[2]))
		return 1;
	} else
	    if(lue_tiedosto(KELLO, ""))
		return 1;
    }
  
    r = käynnistä();

    sulje_aanireuna(aaniputki0, aaniputki1, &poll_aani);
    tuhoa_asetelma();
EI_FONTTI:
    SDL_DestroyTexture(tausta);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(ikkuna);
    TTF_Quit();
EI_TTF:
    SDL_Quit();
EI_SDL:
    return r;
}
