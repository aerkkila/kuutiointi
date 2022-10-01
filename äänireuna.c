#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "äänireuna.h"

static void putki0_tapahtumat();
static void _valinta();
void äänen_valinta(float* kokodata, int raitoja, int raidan_pit, snd_pcm_t* kahva_play, int ulos_fno); // ulkoinen

snd_pcm_t* kahva_capt;
snd_pcm_t* kahva_play;
const int taaj = TAAJ_Hz;
int jaksonaika_ms = 30;
int tallennusaika_ms = 5000;
int gauss_sigma_kpl = 180;
int maksimin_alue = 30; //yhteen suuntaan
float kynnysarvot[] = {NAN, NAN, NAN, 1e-6};
long unsigned pit_data, pit_jakso;
unsigned tarkka_taaj;
snd_pcm_uframes_t buffer_size;
int virhe_id;
float* data[4];
float* kokodata;
enum {raaka, suodate, derivoitu, ohennus, n_raitoja};
#define ALSAFUNK(fun,...) if( (virhe_id = fun(__VA_ARGS__)) < 0 )	\
    {									\
	fprintf(stderr,"\033[31mVirhe funktiossa %s (äänireuna.c):\033[0m %s\n",#fun,snd_strerror(virhe_id)); \
	return virhe_id;						\
    }
static uint32_t kuluma_ms = 0;
static struct pollfd poll_0 = {STDIN_FILENO, POLLIN}; // stdin korvataan tarvittaessa argumentilla

uint32_t opetusviive_ms = -1;
int p00=-1, p11=-1, apuint;
int nauh_jakson_id = 0;
int luet_jakson_id = -1;
int nauh_jaksoja;
int nauh_jatka = 1;
int nauh_tauko = 0;
int nauh_tauolla = 0;
long tallenna_kun = 0;
uint64_t nauhoitteen_loppuhetki = 0;

void sulje_putki(void** putki) {
    close(*((int*)putki));
}

/* komentoriviargumentit */
const struct {char* nimi; char* muoto; void* muuttujat[3]; void (*funktio)(void**); void** funargt;} kntoarg[] = {
    { "--tallennusaika_ms", "%i", {&tallennusaika_ms} },
    { "--jaksonaika_ms",    "%i", {&jaksonaika_ms} },
    { "--opetusviive_ms",   "%i", {&opetusviive_ms} },
    { "--gauss_sigma_kpl",  "%i", {&gauss_sigma_kpl} },
    { "--kynnysarvo",       "%e", {kynnysarvot+3} },
    { "--putki0",           "%i", {&p00, &apuint}, sulje_putki, (void**)&apuint },
    { "--putki1",           "%i", {&apuint, &p11}, sulje_putki, (void**)&apuint },
};

int alusta_ääni(snd_pcm_t** kahva, snd_pcm_stream_t suoratoisto) {
    snd_pcm_hw_params_t* hwparams;
    ALSAFUNK(snd_pcm_open, kahva, "default", suoratoisto, 0);                                  // avaa
    snd_pcm_hw_params_alloca(&hwparams);						          
    ALSAFUNK(snd_pcm_hw_params_any, *kahva, hwparams);                                         // params_any
    ALSAFUNK(snd_pcm_hw_params_set_access, *kahva, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);   // access
    ALSAFUNK(snd_pcm_hw_params_set_rate_resample, *kahva, hwparams, 0);			          
    ALSAFUNK(snd_pcm_hw_params_set_format, *kahva, hwparams, SND_PCM_FORMAT_FLOAT_LE);         // format
    tarkka_taaj = taaj;									          
    ALSAFUNK(snd_pcm_hw_params_set_rate_near, *kahva, hwparams, &tarkka_taaj, 0);              // rate
    if(taaj != tarkka_taaj)								          
	printf("Käytetään näytteenottotaajuutta %i taajuuden %i sijaan\n", tarkka_taaj, taaj);    
    ALSAFUNK(snd_pcm_hw_params_set_channels, *kahva, hwparams, 1);                             // channels
    ALSAFUNK(snd_pcm_hw_params,*kahva,hwparams);
    snd_pcm_hw_params_get_buffer_size(hwparams,&buffer_size);
    return 0;
}

uint64_t hetkinyt() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
}

void _siirrä_dataa_ympäri_vasen(float* data, int siirto, int pit, float* muisti) {
    memcpy(muisti, data, siirto*sizeof(float));
    memmove(data, data+siirto, (pit-siirto)*sizeof(float));
    memcpy(data+pit-siirto, muisti, siirto*sizeof(float));
}

void siirrä_dataa_ympäri_vasen(float* data, int siirto, int pit) {
    if(siirto < 80000) {
	float muisti[siirto];
	_siirrä_dataa_ympäri_vasen(data, siirto, pit, muisti);
	return;
    }
    float* muisti = malloc(siirto*sizeof(float));
    _siirrä_dataa_ympäri_vasen(data, siirto, pit, muisti);
    free(muisti);
}

#define KERROIN 0.39894228 // 1/sqrt(2*pi)
#define GAUSSPAINO(t,sigma) ( KERROIN/sigma * exp(-0.5*(t)*(t)/(sigma*sigma)) )

void suodata(float* data, float* kohde, int pit_data) {
    int gausspit = gauss_sigma_kpl*3;
    float gausskertoimet[gausspit*2+1];
    float* gausskert = gausskertoimet + gausspit; // osoitin keskikohtaan
    for(int t=0; t<=gausspit; t++)
	gausskert[t] = gausskert[-t] = GAUSSPAINO(t,gauss_sigma_kpl);
    for(int i=gausspit; i+gausspit<pit_data; i++) {
	float summa = 0;
	for(int T=-gausspit; T<=gausspit; T++)
	    summa += data[i+T]*data[i+T]*gausskert[T];
	kohde[i] = summa;
    }
}

void derivaatta(float* data, float* kohde, int pit) {
    for(int i=0; i<pit-1; i++)
	kohde[i] = data[i+1]-data[i];
}

void ohentaminen(float* data, float* kohde, int pit) {
    for(int i=maksimin_alue; i+maksimin_alue<pit; i++)
	for(int j=1; j<=maksimin_alue; j++) {
	    if( ! (data[i-j] < data[i] && data[i] > data[i+j]) ) {
		kohde[i] = 0;
		break;
	    }
	    kohde[i] = data[i];
	}
}

void havaitse_ylitykset(float** data, int alkukohta, int pit) {
    suodata( data[raaka]+alkukohta, data[suodate]+alkukohta, pit );
    derivaatta( data[suodate]   + alkukohta+3*gauss_sigma_kpl,
		data[derivoitu] + alkukohta+3*gauss_sigma_kpl,
		pit -= 6*gauss_sigma_kpl );
    ohentaminen( data[derivoitu] + alkukohta+3*gauss_sigma_kpl,
		 data[ohennus]   + alkukohta+3*gauss_sigma_kpl,
		 pit -= 1 );
}

enum tallenn_arg {tallentaminen_tallenna_takaa, tallentaminen_tallenna_tästä, tallentaminen_jälkipuoli,
		  tallentaminen_palauta_tallenne, tallentaminen_palauta_hetki, tallentaminen_vapauta};
void* tallentaminen(int tallenn_arg_enum) {
    static float* tallenne;
    static uint64_t tallenteen_loppuhetki;
    switch(tallenn_arg_enum) {

    case tallentaminen_tallenna_takaa:
	tallenna_kun = 0;
	if(!tallenne) tallenne = malloc(pit_data*sizeof(float));
	int alku = (nauh_jakson_id + 1) % nauh_jaksoja * pit_jakso; // pitäisi vähän siirtää, ettei ehditä kirjoittaa päälle
	tallenteen_loppuhetki = nauhoitteen_loppuhetki;
	memcpy(tallenne, data[raaka]+alku, (pit_data-alku)*sizeof(float));
	memcpy(tallenne+(pit_data-alku), data[raaka], alku*sizeof(float));
	break;

    case tallentaminen_tallenna_tästä:
	tallenna_kun = hetkinyt() + tallennusaika_ms - 100;

    case tallentaminen_palauta_tallenne:
	return tallenne;

    case tallentaminen_palauta_hetki:
	return &tallenteen_loppuhetki;

    case tallentaminen_vapauta:
	free(tallenne);
	tallenne = NULL;
	return NULL;

    }
    return NULL;
}

void sigint(int sig) {
    nauh_jatka = 0;
}
void sigchld(int sig) {
    while(waitpid(-1, NULL, WNOHANG)>0);
}

void* nauhoita(void* datav) {
    float* data = datav;
    nauh_jakson_id = 0;
    snd_pcm_prepare(kahva_capt);
    while(nauh_jatka) {
	if(luet_jakson_id == nauh_jakson_id) {
	    fprintf(stderr, "\033[31mVaroitus:\033[0m kaikkea dataa ei ehditty käsitellä\n");
	    while(luet_jakson_id == nauh_jakson_id)
		usleep(1000);
	}
	if(nauh_tauko) {
	    nauh_tauolla = 1;
	    snd_pcm_drop(kahva_capt); // keskeyttää nauhoituksen äänikortille
	    while(nauh_tauko)
		usleep(2000);
	    snd_pcm_prepare(kahva_capt); // jatkaa nauhoitusta äänikortille
	    nauh_tauolla = 0;
	}
	while(snd_pcm_readi(kahva_capt, data+pit_jakso*nauh_jakson_id, pit_jakso) < 0) { // lukee äänikortilta
	    snd_pcm_prepare(kahva_capt);
	    fprintf(stderr, "Puskurin ylitäyttö\n");
	}
	nauhoitteen_loppuhetki = hetkinyt();
	nauh_jakson_id = (nauh_jakson_id+1) % nauh_jaksoja;
    }
    snd_pcm_drop(kahva_capt);
    return NULL;
}

void käsittele(void* datav) {
    float** data = datav;
    luet_jakson_id = -1;
    while(nauh_jatka) {
	int uusi_id = (luet_jakson_id+1) % nauh_jaksoja;
	while(uusi_id == nauh_jakson_id)
	    usleep(1000);
	luet_jakson_id = uusi_id;
	havaitse_ylitykset(data, pit_jakso*luet_jakson_id, pit_jakso); // jaksojen välit pitää vielä käsitellä
#if 0
	for(int i=0; i<pit_jakso; i++) {
	    int ii = i+pit_jakso*luet_jakson_id;
	    if(data[ohennus][ii] < kynnysarvot[3]) continue;
	    if(p11 < 0) printf("%.3e\n", data[ohennus][ii]);
	    else        write(p11, data[ohennus]+ii, sizeof(float));
	}
#endif
	if((kuluma_ms+=jaksonaika_ms) >= opetusviive_ms) {
	    _valinta();
	    kuluma_ms = 0;
	}
	if(tallenna_kun && hetkinyt() >= tallenna_kun)
	    tallentaminen(tallentaminen_tallenna_takaa);
	putki0_tapahtumat();
    }
}

void _valinta() {
    nauh_tauko = 1;
    while(!nauh_tauolla)
	usleep(2000);
    siirrä_dataa_ympäri_vasen(data[raaka], nauh_jakson_id*pit_jakso, pit_data);
    havaitse_ylitykset(data, 0, pit_data);
    äänen_valinta(kokodata, n_raitoja, pit_data, kahva_play, p11);
    //siirrä_dataa_ympäri_vasen( data[raaka], pit_data-nauh_jakson_id*pit_jakso, pit_data ); //takaisin ennalleen
    for(int i=0; i<pit_data; i++)
	data[raaka][i] = NAN;
    nauh_jakson_id = 0;
    luet_jakson_id = -1;
    nauh_tauko = 0;
}

void katso_viesti(int viesti) {
    switch(viesti) {
    case aanireuna_valinta:
	_valinta();
	return;
    case äänireuna_tallenna_takaa:
	tallentaminen(tallentaminen_tallenna_takaa);
	return;
    case äänireuna_tallenna_tästä:
	tallentaminen(tallentaminen_tallenna_tästä);
	return;
    case aanireuna_valitse_molemmat:
	nauh_tauko = 1;
	float* _data = malloc(pit_data*sizeof(float));
	while(!nauh_tauolla)
	    usleep(1500);
	uint64_t _hetki = nauhoitteen_loppuhetki;
	nauhoitteen_loppuhetki = *(uint64_t*)tallentaminen(tallentaminen_palauta_hetki);
	memcpy(_data, data[raaka], pit_data*sizeof(float));
	float* a = tallentaminen(tallentaminen_palauta_tallenne);
	if(!a) puts("äänen tallenne puuttuu");
	memcpy(data[raaka], a, pit_data*sizeof(float));
	havaitse_ylitykset(data, 0, pit_data);
	äänen_valinta(kokodata, n_raitoja, pit_data, kahva_play, p11); // valitaan alkukohta
	nauhoitteen_loppuhetki = _hetki;
	memcpy(data[raaka], _data, pit_data*sizeof(float));
	siirrä_dataa_ympäri_vasen(data[raaka], nauh_jakson_id*pit_jakso, pit_data);
	havaitse_ylitykset(data, 0, pit_data);
	free(_data);
	äänen_valinta(kokodata, n_raitoja, pit_data, kahva_play, p11); // valitaan loppukohta
	nauh_jakson_id = 0;
	luet_jakson_id = -1,
	    nauh_tauko = 0;
	return;
    }
}

void putki0_tapahtumat() {
    int apu;
    if(!(apu=poll(&poll_0, 1, 0))) return;
    if(apu < 0) {
	fprintf(stderr, "Virhe (äänireuna->putki0_tapahtumat->poll): %s\n", strerror(errno));
	return;
    }
    uint8_t viesti;
    if(!poll_0.revents) return;
    if(poll_0.revents & POLLIN) {
	if((apu=read(poll_0.fd, &viesti, 1)) == 1) katso_viesti(viesti);
	else if(apu < 0)
	    fprintf(stderr, "Virhe putken luennassa (äänireuna->putki0_tapahtumat): %s\n", strerror(errno));
	else
	    fprintf(stderr, "Viestiä ei luettu, vaikka oli muka saatavilla (äänireuna->putki0_tapahtumat)\n");
    }
    else if(poll_0.revents & POLLHUP)
	nauh_jatka = 0;
    else if(poll_0.revents & POLLERR)
	fprintf(stderr, "Virhetila putkessa %i (äänireuna->putki0_tapahtumat)\n", poll_0.fd);
}

void lue_kntoriviargt(int argc, char** argv) {
    int pit = sizeof(kntoarg) / sizeof(kntoarg[0]);
    for(int i=1; i<argc; i++) {
	for(int j=0; j<pit; j++) {
	    if(strcmp(argv[i], kntoarg[j].nimi))
		continue;
	    for(int k=0; kntoarg[j].muuttujat[k]; k++)
		if(sscanf(argv[++i], kntoarg[j].muoto, kntoarg[j].muuttujat[k]) != 1) {
		    fprintf(stderr, "\033[31mVirhe\033[0m argumentin \"%s\" jälkeen\n", kntoarg[j].nimi);
		    exit(1);
		}
	    if(kntoarg[j].funktio)
		kntoarg[j].funktio(kntoarg[j].funargt);
	    goto seuraava_argumentti;
	}
	fprintf(stderr, "\033[31mTuntematon argumentti:\033[0m %s\n", argv[i]);
	exit(1);
    seuraava_argumentti:
    }
}

int main(int argc, char** argv) {
    lue_kntoriviargt(argc, argv);
    if(p00>0)
	poll_0.fd = p00;
    signal(SIGINT, sigint);
    signal(SIGPIPE, sigint);
    signal(SIGCHLD, sigchld);
    pthread_t saie;
    pit_data = taaj*tallennusaika_ms/1000;
    pit_jakso = taaj*jaksonaika_ms/1000;
    nauh_jaksoja = pit_data / pit_jakso;
    if(nauh_jaksoja < 2) {
	printf("\033[31mVaroitus:\033[0m tallennusaikaa täytyi pidentää.\n");
	nauh_jaksoja = 2;
    }
    pit_data = nauh_jaksoja * pit_jakso; // tehtäköön tästä jakson pituuden monikerta
    alusta_ääni(&kahva_capt, SND_PCM_STREAM_CAPTURE);
    alusta_ääni(&kahva_play, SND_PCM_STREAM_PLAYBACK);
    kokodata = calloc(pit_data * n_raitoja, sizeof(float));
    for(int i=0; i<n_raitoja; i++)
	data[i] = kokodata + i*pit_data;

    for(int i=0; i<pit_data*n_raitoja; i++)
	kokodata[i] = NAN;

    pthread_create(&saie, NULL, nauhoita, kokodata);
    käsittele(data);
    pthread_join(saie, NULL);
    snd_pcm_close(kahva_capt);
    close(p00);
    p00 = -1;
    poll_0.fd = -1;
    if(p11 >= 0) {
	close(p11);
	p11 = -1;
    }
    free(kokodata);
    tallentaminen(tallentaminen_vapauta);
    puts("\nÄäniohjelma lopetti");
    return 0;
}
