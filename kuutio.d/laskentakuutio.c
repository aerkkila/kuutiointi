/*
  Lasketaan monellako siirrolla pääsee takaisin alkuun toistamalla jotain yhdistelmää.
 
  1. siirto on aina r[], koska kuutiota kääntämällä muut siirrot voidaan muuntaa siksi.
  2. siirto on aina l[] tai u[], koska kuutiota kääntämällä muut voidaan muuttaa niiksi.
  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <err.h>
#include <unistd.h>
#include <sys/time.h>

#include "kuutio.c"

static const char* tahkot = "RUFLDB";
static const char* suunnat = " '2";
static const int isuunnat[] = {1,3,2}; // montako kertaa 90° myötäpäivään mikin suunta on
static const char* suuntakirjaimet_90aste = "@ 2'";
typedef long unsigned uint64;

void seuraava_sarja(uint64 *sexa, uint64 *trexa, int pit, int* isarja, long raja);
void korjaa_sarja  (uint64 *sexa, uint64 *trexa, int pit, int* isarja, long raja);
uint64 viimeinen_sexa(uint64 sexa, int pit);
uint64 ensimmäinen_sexa(uint64 raja, int pit);
void* laskenta(void* vp);

uint64 powi(uint64 a, uint64 n);
void stp_sarjaksi(uint64, uint64, int, char* ulos);
char* isarja_sarjaksi(int*, int, char*);

struct Lista {
    size_t pit, kapasit;
    unsigned *lasku, *kutakin;
};

typedef struct ST {
    struct Lista* lista;
    uint64 alku;
    long raja;
    int pit;
    int id;
    void(*tulostfun)(struct ST*, long, long);
} säikeen_tiedot;

/* Merkittäköön siirtotahkot kuusikantaisilla sexaluvuilla.
   esim. 0135 olisi R U L B
   lisättäessä 1 saadaan 0140 eli R U D R
   Vastaavasti suunnat merkitään kolmikantaisilla trexaluvuilla.

   Kun luku esitetään kanta-kantaisena, mikä on vähiten merkitsevästä alkaen i. numero.
   Jakojäännös noukkii i+1 viimeistä lukua, esim (kanta=10,i=1). 1215 % 10^2 -> 15.
   Jakolasku noukkii i:nnen luvun: 15 / 10^1 -> 1 */
#define NLUKU(luku, i, kanta) ((luku) % powi(kanta,i+1) / powi(kanta,i))

uint64 powi(uint64 a, uint64 n) { // tämä toimii ajassa O(log(n))
    uint64 r=1;
alku:
    if(!n) return r;
    if(n%2) r *= a;
    n /= 2;
    a *= a;
    goto alku;
}

#define EI_LOPUSSA(työ) (ind[työ] < listat[työ].pit)
void vie_tiedostoksi(struct Lista* listat, int töitä, const char* nimi) {
    FILE *f = fopen(nimi, "w");
    if(!f) {
	err(1, "fopen funktiossa vie_tiedostoksi");
	return; }
    fprintf(f, "siirtoja kpl\n");
    size_t ind[töitä];
    memset(ind, 0, töitä*sizeof(size_t));
    unsigned pienin, n_eilopussa;
    do {
	pienin = (unsigned)-1;
	n_eilopussa = 0;
	for(int i=0; i<töitä; i++)
	    if(EI_LOPUSSA(i)) {
		n_eilopussa++;
		if(listat[i].lasku[ind[i]] < pienin)
		    pienin = listat[i].lasku[ind[i]];
	    }
	if(pienin == -1)
	    break;

	unsigned summa = 0;
	for(int i=0; i<töitä; i++)
	    if(EI_LOPUSSA(i) && listat[i].lasku[ind[i]] == pienin)
		summa += listat[i].kutakin[ind[i]++];

	fprintf(f, "%-8u %u\n", pienin, summa);
    } while(n_eilopussa > 1);
    /* Enintään yhtä työtä on jäljellä. */
    int lasku = 0;
    for(int i=0; i<töitä; i++)
	if(EI_LOPUSSA(i)) {
	    if(lasku)
		printf("Varoitus %s: rivi %i\n", __FILE__, __LINE__);
	    for(int j=ind[i]; j<listat[i].pit; j++)
		fprintf(f, "%-8u %u\n", listat[i].lasku[j], listat[i].kutakin[j]);
	    lasku++;
	}
    fclose(f);
}
#undef EI_LOPUSSA

long *kunkin_tila;
uint64 *rajan_anto, kaikkiaan_glob;
int töitä=1, verbose=0;
short* volatile valmis;

/* Operaattori &&, jolla otetaan viite goto-merkkiin on GNU-laajennos,
   ja monen mielestä huonoa tyyliä,
   mutta hähää, minullapa onkin vapaus kirjoittaa sellaista koodia kuin haluan.
   Ei pidä vain seurata jotain kieltoja, vaan pitää katsoa, onko koodi luettavaa ja toimivaa.
   Samaa voisi soveltaa liikennevaloihin: ei pidä vain katsoa onko valo punainen,
   vaan jos ketään ei tule, niin kyllä siitä pitäisi olla silloin lupa mennä.
   */
int antakaa_työtä(säikeen_tiedot* tied) {
    void* odottaminen_ = &&odottaminen;
    void* saaminen_ = &&saaminen;
    valmis[tied->id] = 1;
odottaminen:
    usleep(5000);
    for(int i=0; i<töitä; i++)
	if(valmis[i] != 1)
	    goto *(!valmis[tied->id]? saaminen_: odottaminen_);
    return 0;
saaminen:
    tied->alku = rajan_anto[tied->id*2];
    tied->raja = rajan_anto[tied->id*2+1];
    return 1;
}

int anna_työtä(säikeen_tiedot* tied, uint64 sexa, int säie) {
    if(valmis[säie] != 1) // funktion kutsun aikana jokin muu säie on voinut jo ehtiä varata tämän
	return 1;
    valmis[säie] = tied->id+2; // varataan antamisvuoro tälle säikeelle
    /* Yllä olevan ehtolauseen ja kirjoituksen välissä jokin muukin säie on voinut todeta tämän olevan vapaana,
       mutta ehtiä kirjoittamaan varauksen vasta hetken kuluttua.
       Odotettakoon siksi hetki ja tarkkailtakoon, pysyykö varaus voimassa.
       */
    for(int i=0; i<20; i++)
	if(valmis[säie] != tied->id+2)
	    return 1;
    uint64 luku = (sexa+1+tied->raja) / 2;
    rajan_anto[säie*2] = luku;
    rajan_anto[säie*2+1] = tied->raja;
    valmis[säie] = 0;
    tied->raja = luku;
    return 0;
}

double hetkinyt() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec*1e-6;
}

void tiedotusfun(säikeen_tiedot* tied, long käytyjä, long sivuttuja) {
    kunkin_tila[tied->id] = käytyjä;
    kunkin_tila[tied->id+töitä] = sivuttuja;
}

/* Ensimmäinen säie saa tämän ja loput tiedotusfun-funktion. */
void tulostfun(säikeen_tiedot* tied, long käytyjä, long sivuttuja) {
    static double viimeaika = 0;
    double nyt = hetkinyt();
    if(nyt-viimeaika < 0.3)
	return;
    viimeaika = nyt;
    for(int i=1; i<töitä; i++) {
	käytyjä += kunkin_tila[i];
	sivuttuja += kunkin_tila[i+töitä];
    }
    if(!käytyjä || !sivuttuja)
	return;
    long kaikkiaan = kaikkiaan_glob - (sivuttuja-käytyjä);
    printf("\033[Ksivuttu %li/%li -> %.2lf %%, käyty %li/≤%lu -> ≥%.2lf %%\r",
	    sivuttuja, kaikkiaan_glob, (double)sivuttuja/kaikkiaan_glob*100, käytyjä, kaikkiaan, (double)käytyjä/kaikkiaan*100);
    fflush(stdout);
}

void nop(){}

/* Käytetään osiossa 'polkuosio'. */
#define R(tahko,j,i) SIVU2(3*3, 3, tahko, j, i)
const int nurkasta_indeksi[] = {
    R(_r,0,0), R(_r,0,2), R(_r,2,0), R(_r,2,2), R(_l,0,0), R(_l,0,2), R(_l,2,0), R(_l,2,2),
    R(_f,0,2), R(_u,0,2), R(_d,0,2), R(_b,2,0), R(_b,0,2), R(_u,2,0), R(_d,2,0), R(_f,2,0),
    R(_u,2,2), R(_b,0,0), R(_f,2,2), R(_d,2,2), R(_u,0,0), R(_f,0,0), R(_b,2,2), R(_d,0,0)
};
const int reunasta_indeksi[] = {
    R(_r,0,1), R(_r,1,2), R(_r,2,1), R(_r,1,0), R(_l,0,1), R(_l,1,2), R(_l,2,1), R(_l,1,0), R(_u,0,1), R(_u,2,1), R(_d,0,1), R(_d,2,1),
    R(_u,1,2), R(_b,1,0), R(_d,1,2), R(_f,1,2), R(_u,1,0), R(_f,1,0), R(_d,1,0), R(_b,1,2), R(_b,0,1), R(_f,0,1), R(_f,2,1), R(_b,2,1)
};
int indeksistä_nurkka[3*3 * 6] = {-1};
int indeksistä_reuna[3*3 * 6] = {-1};

void aseta_hakuindeksit() {
    for(int i=0; i<54; i++)
	for(int n=0; n<24; n++)
	    if(nurkasta_indeksi[n] == i) {
		indeksistä_nurkka[i] = n;
		break;
	    }
    for(int i=0; i<54; i++)
	for(int n=0; n<24; n++)
	    if(reunasta_indeksi[n] == i) {
		indeksistä_reuna[i] = n;
		break;
	    }
}

int main(int argc, char** argv) {
    aseta_hakuindeksit();
    /*komentoriviargumentit*/
    int maxpit_l = 4;
    int minpit_l = -1;
    if(argc > 1)
	sscanf(argv[1], "%i", &maxpit_l);
    for(int i=1; i<argc; i++)
	if(!strcmp(argv[i], "--alkaen") || !strcmp(argv[i], "-a")) {
	    if(sscanf(argv[++i], "%i", &minpit_l) != 1)
		warn("sscanf minpit_l epäonnistui");
	}
	else if(!strcmp(argv[i], "--töitä") || !strcmp(argv[i], "-j")) {
	    if(sscanf(argv[++i], "%i", &töitä) != 1)
		warn("sscanf töitä epäonnistui");
	}
	else if(!strcmp(argv[i], "-v"))
	    verbose = 1;
    if(minpit_l > maxpit_l)
	maxpit_l = minpit_l;
    if(minpit_l < 0)
	minpit_l = maxpit_l; // oletuksena tehdään vain yksi
    pthread_t säikeet[töitä];
    long _kunkin_tila[töitä*2];
    short _valmis[töitä];
    uint64 _rajan_anto[töitä*2];
    kunkin_tila = _kunkin_tila;
    valmis = _valmis;
    rajan_anto = _rajan_anto;

    for(int pit=minpit_l; pit<=maxpit_l; pit++) {
	memset(kunkin_tila, 0, töitä*sizeof(int));
	memset(valmis, 0, sizeof(_valmis));
	struct Lista listat[töitä];
	uint64 sexa1 = _d*powi(6, pit-2); // Lopetetaan, kun toinen siirto olisi L+1=D, jolloin kaikki on käyty.
	sexa1 = viimeinen_sexa(sexa1, pit); // Tarkemmin päätepiste jäljellä olevan ajan arvioimiseksi.
	uint64 sexa0 = ensimmäinen_sexa(sexa1, pit);
	kaikkiaan_glob = sexa1-sexa0;
	uint64 pätkä = kaikkiaan_glob/töitä;
	säikeen_tiedot t[töitä];
	void(*funptr)(säikeen_tiedot*,long,long) = verbose? nop: tulostfun; // huomattakoon käänteisyys
	int i;
	for(i=0; i<töitä-1; i++) {
	    listat[i] = (struct Lista){0};
	    t[i] = (säikeen_tiedot) {
		.lista     = listat+i,
		.alku      = pätkä*i     + sexa0,
		.raja      = pätkä*(i+1) + sexa0,
		.pit       = pit,
		.id        = i,
		.tulostfun = funptr
	    };
	    pthread_create(säikeet+i, NULL, laskenta, t+i);
	    funptr = verbose? nop: tiedotusfun;
	}
	listat[i] = (struct Lista){0};
	t[i] = (säikeen_tiedot){.lista=listat+i, .alku=pätkä*i+sexa0, .raja=sexa1, .pit=pit, .id=i, .tulostfun=funptr};
	laskenta(t+töitä-1);
	for(int i=0; i<töitä-1; i++)
	    pthread_join(säikeet[i], NULL);

	char nimi[20];
	sprintf(nimi, "sarjat%i.txt", pit);
	vie_tiedostoksi(listat, töitä, nimi);

	for(int i=0; i<töitä; i++) {
	    free(listat[i].lasku);
	    free(listat[i].kutakin);
	}
    }
    if(!verbose)
	write(STDOUT_FILENO, "\033[K", 3);
}

#ifdef DEBUG
void stp_sarjaksi(uint64 sexa, uint64 trexa, int pit, char* sarja) {
    for(int i=0; i<pit; i++) {
	sarja[(pit-i-1)*3] = tahkot[NLUKU(sexa, i, 6)];
	sarja[(pit-i-1)*3+1] = suunnat[NLUKU(trexa, i, 3)];
	sarja[(pit-i-1)*3+2] = ' ';
    }
    sarja[pit*3] = '\0';
}

/* Voidaan kutsua vuorovaikutuksellisesti virheenjäljittäjästä.
   Muuten tätä ei käytetä. */
void tulosta_sarja_stp(uint64 sexa, uint64 trexa, int pit) {
    char sarja[64];
    stp_sarjaksi(sexa, trexa, pit, sarja);
    puts(sarja);
}
#endif

char* isarja_sarjaksi(int* isarja, int pit, char* sarja) {
    for(int i=0; i<pit; i++) {
	sarja[(pit-i-1)*3] = tahkot[isarja[(pit-i-1)*2]];
	sarja[(pit-i-1)*3+1] = suuntakirjaimet_90aste[isarja[(pit-i-1)*2+1]];
	sarja[(pit-i-1)*3+2] = ' ';
    }
    sarja[pit*3] = '\0';
    return sarja;
}

size_t puolitushaku(unsigned* a, size_t pit, unsigned kohde) {
    size_t i0 = 0, i1=pit, keski;
alku:
    if(i1-i0 < 11) {
	for(size_t i=i0; i<i1; i++)
	    if(kohde <= a[i]) {
		return i;
	    }
	return i1;
    }
    keski = (i0 + i1) / 2;
    if(kohde < a[keski])
	i1 = keski;
    else if(kohde > a[keski])
	i0 = keski+1;
    else
	return keski;
    goto alku;
}

/* Kutsuttakoon ainoastaan funktiosta luku_listalle. */
int _jatka_listaa(struct Lista* lista) {
    lista->lasku   = realloc(lista->lasku,   (lista->kapasit+=1024)*sizeof(unsigned));
    lista->kutakin = realloc(lista->kutakin, (lista->kapasit)      *sizeof(unsigned));
    return !(lista->lasku && lista->kutakin);
}

void _luku_listalle(int lasku, int ind, struct Lista* lista) {
    lista->lasku[ind] = lasku;
    lista->kutakin[ind] = 1;
    lista->pit++;
}

int luku_listalle(int lasku, struct Lista* lista) {
    if(!lista->kapasit) {
	if(_jatka_listaa(lista))
	    return 1;
	_luku_listalle(lasku, 0, lista);
	return 0;
    }

    size_t ind = puolitushaku(lista->lasku, lista->pit, lasku);
    if(ind < lista->pit && lista->lasku[ind] == lasku) {
	lista->kutakin[ind]++;
	return 0;
    }
    if(lista->pit+1 > lista->kapasit)
	if(_jatka_listaa(lista))
	    return 1;
    memmove(lista->lasku+ind+1,   lista->lasku+ind,   (lista->pit-ind)*sizeof(unsigned));
    memmove(lista->kutakin+ind+1, lista->kutakin+ind, (lista->pit-ind)*sizeof(unsigned));
    _luku_listalle(lasku, ind, lista);
    return 0;
}

/* isarjassa parilliset indeksit ovat tahkoja */
int yhtenevyys_aiempaan(int* isarja, int pit) {
    enum {x,y,z};
    char puhdas_aks[3] = {1,1,1};
    char puhdas_suunta[3] = {1,1,1};
    int i;

    int _tarkistus() {
	int ax = isarja[i] % 3;
	if(puhdas_aks[ax] && puhdas_aks[x]) {
	    if(ax != x)
		return 1; // Akseli voidaan kääntää x-akseliksi. Ei kyllä tapahdu, koska ensimmäinen on aina R.
	}
	else if(puhdas_aks[ax] && puhdas_aks[y]) {
	    if(ax != y)
		return 1; // Akseli voidaan kääntää y-akseliksi.
	}
	puhdas_aks[ax] = 0;

	if(puhdas_suunta[x]+puhdas_suunta[y]+puhdas_suunta[z] <= 1) // yhdestä vapaasta suunnasta ei ole hyötyä
	    return 0;
	if(puhdas_suunta[ax] && isarja[i] >= 3) // Akseli voi kääntyä 180°, yllä jo tarkistettiin, että toinenkin vapaa löytyy.
	    return 1;

	if(isarja[i+2]%3 != ax)
	    puhdas_suunta[ax] = 0;
	else
	    i+=2; // Saman akselin käsittely uudestaan ei hyödytä ja sotkisi puhtaat suunnat.
	return 0;
    }

    for(i=0; i<(pit-1)*2; i+=2)
	if(_tarkistus())
	    return 1;
    if(i<pit*2)
	return _tarkistus();
    return 0;
}

/* isarja muodostetaan kutsumalla nämä kaksi funktiota.
   Ne ovat erillään, koska sexa-osiota ei tarvitse aina päivittää.
   sexa_isarjaan kutsuttakoon ainoastaan funktiosta korjaa_sexa
   Yhdistelmä riittävissä määrin kutsutaan funktiosta seuraava_sarja.
   */
void sexa_isarjaan(int* isarja, uint64 sexa, int pit) {
    for(int i=0; i<pit; i++) {
	int ind = NLUKU(sexa, i, 6);
	isarja[(pit-i-1)*2] = ind;
    }
}
void trexa_isarjaan(int* isarja, uint64 trexa, int pit) {
    if(trexa == 0)
	for(int i=0; i<pit; i++)
	    isarja[i*2+1] = isuunnat[0];
    else
	for(int i=0; i<pit; i++) {
	    int ind = NLUKU(trexa, i, 3);
	    isarja[(pit-i-1)*2+1] = isuunnat[ind];
	}
}

int gcd(int a, int b) {
    while(b) {
	int apu = b;
	b = a % b;
	a = apu;
    }
    return a;
}

int lcm(int a, int b) {
    if(a<b) {
	a ^= b; b ^= a; a ^= b; }
    return a*b / gcd(a,b);
}

int lcm_(int* luvut, int pit) {
    if(pit < 2)
	return pit>0? luvut[0]: 0;
    int ret = lcm(luvut[0], luvut[1]);
    for(int i=2; i<pit; i++)
	ret = lcm(ret, luvut[i]);
    return ret;
}

#define PIT_SARJA (pit*3)
#define PIT_ISARJA (pit*2)
void* laskenta(void* vp) {
    kuutio_t kuutio;
    luo_kuutio(&kuutio, 3);
    säikeen_tiedot tied = *(säikeen_tiedot*)vp; // Miksi tämä kopioidaan?
    int pit = tied.pit;
    char sarja[PIT_SARJA+1];
    int  isarja[PIT_ISARJA];
    memset(sarja, suunnat[0], PIT_SARJA+1);
    for(int i=0; i<pit*2; i++)
	isarja[i] = isuunnat[0];
    sarja[PIT_SARJA] = '\0';

    char nimi[12];
    sprintf(nimi, "tmp%i.txt", tied.id);
    uint64 trexa = 0, sexa = tied.alku;
    korjaa_sarja(&sexa, &trexa, pit, isarja, tied.raja);
    trexa--; // jotta seuraava_sarja palauttaakin saman
    if(sexa > tied.raja)
	sexa = tied.raja;
    long käytyjä = 0, sivuttuja = sexa-tied.alku;
    const int pötkö = 200;

    assert(kuutio.N2*6 <= 256);
    char muutos[kuutio.N2*6];
    char* apusivut = malloc(kuutio.N2*6);

    char nurkan_maski[8];
    char reunan_maski[12];
    char käytetyt_pituudet[25];
#undef R

    while(1) {
	for(int i=0; i<pötkö; i++) {
	    uint64 vanha = sexa;
	    seuraava_sarja(&sexa, &trexa, pit, isarja, tied.raja);
	    /* Jos on valmis, pyydetään muilta säikeiltä lisää ja lopetetaan ellei saada. */
	    while(sexa >= tied.raja) {
		sivuttuja += tied.raja-vanha; // kaikkea on nyt sivuttu
		if(!antakaa_työtä(&tied))
		    goto ulos;
		sexa = tied.alku;
		vanha = sexa;
		trexa = 0;
		korjaa_sarja(&sexa, &trexa, pit, isarja, tied.raja);
	    }
	    sivuttuja += sexa-vanha;
	    käytyjä += sexa!=vanha;

#if 1
	    /* Tarkistetaan, mitä tämä sarja tekee. */
	    char* apu = kuutio.sivut;
	    kuutio.sivut = muutos;
	    for(int i=0; i<kuutio.N2*6; i++)
		kuutio.sivut[i] = i;
	    for(int i=0; i<pit; i++)
		siirto(&kuutio, isarja[i*2], 0, isarja[i*2+1]);
	    kuutio.sivut = apu;

	    /* Lasketaan tarvittavat toistot */
#if 0
	    /* Tässä tehdään sarja kerrallaan käyttäen edellä luotuja indeksejä. */
	    int lasku = 0;
	    do {
		for(int i=0; i<kuutio.N2*6; i++)
		    apusivut[i] = kuutio.sivut[(int)muutos[i]];
		apu=apusivut; apusivut=kuutio.sivut; kuutio.sivut=apu;
		lasku++;
	    } while(!onkoRatkaistu(&kuutio));
	    lasku *= pit;
#else
	    /* Polkuosio: Tämä on ylivertaisesti nopein menetelmä.
	       Tässä ei tehdä siirtoja ollenkaan,
	       vaan analysoidaan kunkin palan kulkema polku
	       ja otetaan niitten pituuksista pienin yhteinen kertoja. */
	    memset(reunan_maski, 0, 12);
	    memset(nurkan_maski, 0, 8);
	    memset(käytetyt_pituudet, 0, 24);
	    int pituudet[20], kierroksia=0;
	    /* reunat */
	    for(int pala=0; pala<12; pala++) {
		if(reunan_maski[pala])
		    continue;
		int kierrospit = 0;
		int tämä = reunasta_indeksi[pala];
		const int pala1 = pala + 12;
		reunan_maski[pala] = 1;
		while(1) {
		    kierrospit++;
		    if(muutos[tämä] == reunasta_indeksi[pala]) // jos 'tämä' on lähtöisin aloitussijainnista
			break; // takaisin alussa
		    if(muutos[tämä] == reunasta_indeksi[pala1]) {
			kierrospit *= 2; // takaisin alussa, mutta väärin päin
			break;
		    }
		    tämä = muutos[tämä];
		    /* Nyt muutos[tämä] on alunperin muutos[muutos[tämä]].
		       Siirretään siis huomio siihen kohtaan, josta äsken tarkasteltu pala oli lähtöisin.
		       Silmukka siis kierretään takaperin. */
		    reunan_maski[indeksistä_reuna[tämä]%12] = 1;
		}
		if(kierrospit > 1 && !käytetyt_pituudet[kierrospit]) {
		    pituudet[kierroksia++] = kierrospit;
		    käytetyt_pituudet[kierrospit] = 1;
		}
	    }
	    /* nurkat */
	    for(int pala=0; pala<8; pala++) {
		if(nurkan_maski[pala])
		    continue;
		int kierrospit = 0;
		int tämä = nurkasta_indeksi[pala];
		const int pala1=pala+8, pala2=pala+16;
		nurkan_maski[pala] = 1;
		while(1) {
		    kierrospit++;
		    if(muutos[tämä] == nurkasta_indeksi[pala])
			break; // takaisin alussa
		    if(muutos[tämä] == nurkasta_indeksi[pala1] ||
		       muutos[tämä] == nurkasta_indeksi[pala2]) {
			kierrospit *= 3; // takaisin alussa, mutta väärin päin
			break;
		    }
		    tämä = muutos[tämä];
		    nurkan_maski[indeksistä_nurkka[tämä]%8] = 1;
		}
		if(kierrospit > 1 && !käytetyt_pituudet[kierrospit]) {
		    pituudet[kierroksia++] = kierrospit;
		    käytetyt_pituudet[kierrospit] = 1;
		}
	    }
	    int lasku = lcm_(pituudet, kierroksia) * pit;
#endif
#else
	    /* Tämä on kaikista yksinkertaisin ja hitain menetelmä:
	       Tehdään siirto kerrallaan ja tarkistetaan onko ratkaistu. */
	    unsigned kohta=0;
	    int lasku=0;
	    do {
		siirto(&kuutio, isarja[kohta*2], 0, isarja[kohta*2+1]);
		kohta = (kohta+1) % pit;
		lasku++;
	    } while(!onkoRatkaistu(&kuutio));
#endif

	    if(luku_listalle(lasku, tied.lista)) {
		puts("epäonnistui");
		return NULL; }
	    if(verbose) {
		isarja_sarjaksi(isarja, pit, sarja);
		printf("%-6li%-6li %s\t%i\n", sexa, trexa, sarja, lasku);
	    }
	}
	tied.tulostfun(&tied, käytyjä, sivuttuja);
	/* Jos jokin muu säie on valmis, annetaan sille tästä osa, ellei jokin muu säie ehdi ensin. */
	for(int i=0; i<töitä; i++)
	    if(valmis[i] == 1 && !anna_työtä(&tied, sexa, i))
		break;
    }

ulos:
    free(kuutio.sivut);
    free(*kuutio.indeksit);
    free(apusivut);
    return NULL;
}

/* Sama tahko ei saa esiintyä kahdesti peräkkäin eikä sama akseli kolmesti. */
int sexa_ei_kelpaa(uint64 sexa, int pit) {
    int n0, n1, n2;
    n0 = NLUKU(sexa, 0, 6);
    n1 = NLUKU(sexa, 1, 6);
    for(int i=0; i<pit-2; i++) {
	n2 = NLUKU(sexa, i+2, 6);
	if(n0 == n1)                     return 1;
	if(n0%3 == n1%3 && n0%3 == n2%3) return 1;
	n0 = n1;
	n1 = n2;
    }
    if(n0 != _u && n0 != _l) return 1; // n0 on nyt toinen siirto, joka on aina U tai L
    return (n0 == n1);
}

/* Muuttujan pit nimen muuttaminen rikkoisi PIT_SARJA-makron.
   Kutsuttakoon vain funktioista seuraava_sarja ja korjaa_sarja. */
uint64 _korjaa_sexa(uint64 sexa, int pit, int* isarja, long raja) {
    while(sexa < raja) {
	while(sexa_ei_kelpaa(sexa, pit)) sexa++;
	if(!(sexa < raja))
	    break;
	sexa_isarjaan(isarja, sexa, pit);
	if(!yhtenevyys_aiempaan(isarja, pit))
	    return sexa;
	if(verbose) {
	    char sarja[PIT_SARJA+1];
	    isarja_sarjaksi(isarja, pit, sarja);
	    printf("%s yhtenevä\n", sarja);
	}
	sexa++;
    }
    return sexa;
}

/* Nämä kaksi ovat vähän kuin yllä oleva _korjaa_sexa,
   mutta näitä käytetään vain alussa pääfunktiossa määrittämään rajat,
   jotta osataan paremmin arvioida jäljellä oleva aika. */
uint64 viimeinen_sexa(uint64 sexa, int pit) {
    int isarja[PIT_ISARJA];
    for(; sexa; sexa--) {
	while(sexa_ei_kelpaa(sexa, pit)) sexa--;
	sexa_isarjaan(isarja, sexa, pit);
	if(!yhtenevyys_aiempaan(isarja, pit))
	    return sexa+1;
    }
    return 0;
}
uint64 ensimmäinen_sexa(uint64 raja, int pit) {
    int isarja[PIT_ISARJA];
    for(uint64 sexa=0; sexa<raja; sexa++) {
	while(sexa_ei_kelpaa(sexa, pit)) sexa++;
	sexa_isarjaan(isarja, sexa, pit);
	if(!yhtenevyys_aiempaan(isarja, pit))
	    return sexa;
    }
    return raja;
}
#undef PIT_SARJA
#undef PIT_ISARJA

void seuraava_sarja(uint64 *sexa, uint64 *trexa, int pit, int* isarja, long raja) {
    if(++*trexa < powi(3, pit))
	return trexa_isarjaan(isarja, *trexa, pit);
    trexa_isarjaan(isarja, *trexa=0, pit);
    *sexa = _korjaa_sexa(++*sexa, pit, isarja, raja);
}

void korjaa_sarja(uint64 *sexa, uint64 *trexa, int pit, int* isarja, long raja) {
    *sexa = _korjaa_sexa(*sexa, pit, isarja, raja);
    trexa_isarjaan(isarja, *trexa, pit);
}
