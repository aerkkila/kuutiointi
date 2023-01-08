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

#include "kuutio.c"

static const char* tahkot = "RUFLDB";
static const char* suunnat = " '2";
static const int isuunnat[] = {1,3,2}; // montako kertaa 90° myötäpäivään mikin suunta on
static const char* suuntakirjaimet_90aste = "@ 2'";
typedef long unsigned uint64;

uint64 korjaa_sexa(uint64, int);
void seuraava_sarja(int pit, uint64 *sexa, uint64 *trexa);
void* laskenta(void* vp);

uint64 powi(uint64 a, uint64 n);
void stp_sarjaksi(uint64, uint64, int, char* ulos);
void isarja_sarjaksi(int*, int, char*);

struct Lista {
    unsigned *lasku, *kutakin;
    size_t pit, kapasit;
};

typedef struct ST {
    struct Lista* lista;
    uint64 sexa;
    long raja;
    int pit;
    int id;
    void(*tulostfun)(struct ST*, long);
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

int *kunkin_tila, töitä=1, verbose=0;
short* volatile valmis;
uint64* rajan_anto;

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
    tied->sexa = rajan_anto[tied->id*2];
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

/* Ensimmäinen säie saa tämän ja loput tiedotusfun-funktion. */
void tulostfun(säikeen_tiedot* tied, long käytyjä) {
    long yht = käytyjä;
    for(int i=1; i<töitä; i++)
	yht += kunkin_tila[i];
    printf("\033[K%li\r", yht);
    fflush(stdout);
}

void tiedotusfun(säikeen_tiedot* tied, long käytyjä) {
    kunkin_tila[tied->id] = käytyjä;
}

void nop(){}

int main(int argc, char** argv) {
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
    int _kunkin_tila[töitä];
    short _valmis[töitä];
    uint64 _rajan_anto[töitä*2];
    kunkin_tila = _kunkin_tila;
    valmis = _valmis;
    rajan_anto = _rajan_anto;
    memset(kunkin_tila, 0, töitä*sizeof(int));

    for(int pit=minpit_l; pit<=maxpit_l; pit++) {
	struct Lista listat[töitä];
	uint64 sexa1 = _d*powi(6, pit-2); // Lopetetaan, kun toinen siirto olisi L+1=D, jolloin kaikki on käyty.
	uint64 pätkä = sexa1/töitä;
	säikeen_tiedot t[töitä];
	void(*funptr)(säikeen_tiedot*,long) = verbose? nop: tulostfun; // verbose-tilassa on muuta tulostettavaa ja tämä väistyy
	int i;
	for(i=0; i<töitä-1; i++) {
	    listat[i] = (struct Lista){0};
	    t[i] = (säikeen_tiedot){.lista=listat+i, .sexa=pätkä*i, .raja=pätkä*(i+1), .pit=pit, .id=i, .tulostfun=funptr};
	    pthread_create(säikeet+i, NULL, laskenta, t+i);
	    funptr = verbose? nop: tiedotusfun;
	}
	listat[i] = (struct Lista){0};
	t[i] = (säikeen_tiedot){.lista=listat+i, .sexa=pätkä*i, .raja=sexa1, .pit=pit, .id=i, .tulostfun=funptr};
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
    write(STDOUT_FILENO, "\033[K", 3);
}

#ifdef DEBUG
void stp_sarjaksi(uint64 sexa, uint64 trexa, int pit, char* sarja) {
    for(int i=0; i<pit; i++) {
	sarja[(pit-i-1)*3] = tahkot[NLUKU(sexa, i, 6)];
	sarja[(pit-i-1)*3+1] = suunnat[NLUKU(trexa, i, 3)];
	sarja[(pit-i-1)*3+2] = ' ';
    }
}

/* Voidaan kutsua vuorovaikutuksellisesti virheenjäljittäjästä.
   Muuten tätä ei käytetä. */
void tulosta_sarja_stp(uint64 sexa, uint64 trexa, int pit) {
    char sarja[64];
    stp_sarjaksi(sexa, trexa, pit, sarja);
    puts(sarja);
}
#endif

void isarja_sarjaksi(int* isarja, int pit, char* sarja) {
    for(int i=0; i<pit; i++) {
	sarja[(pit-i-1)*3] = tahkot[isarja[(pit-i-1)*2]];
	sarja[(pit-i-1)*3+1] = suuntakirjaimet_90aste[isarja[(pit-i-1)*2+1]];
    }
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

#define PIT_SARJA (pit*3)
#define PIT_ISARJA (pit*2*sizeof(int))
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
    uint64 trexa = -1;
    uint64 sexa = korjaa_sexa(tied.sexa, pit);
    long käytyjä = 0, sivuttuja = 0; // sivuttuja toiminee vasta, kun yhtenevyys_aiempaan siirretään korjaa-sexaan
    const int pötkö = 200;

    while(1) {
	for(int i=0; i<pötkö; i++) {
	    uint64 vanha = sexa;
	    seuraava_sarja(pit, &sexa, &trexa);
	    /* Jos on valmis, pyydetään muilta säikeiltä lisää ja lopetetaan ellei saada. */
	    while(sexa >= tied.raja) {
		if(!antakaa_työtä(&tied))
		    goto ulos;
		vanha = sexa;
		sexa = korjaa_sexa(tied.sexa, pit);
	    }
	    sivuttuja += sexa-vanha;

	    /*sarja lukumuotoon*/
	    for(int i=0; i<pit; i++) {
		int ind = NLUKU(sexa, i, 6);
		isarja[(pit-i-1)*2] = ind;
		ind = NLUKU(trexa, i, 3);
		isarja[(pit-i-1)*2+1] = isuunnat[ind];
	    }
	    /* Tämä olisi looginen siirtää korjaa-sexa funktioon. */
	    if(yhtenevyys_aiempaan(isarja, pit)) {
		if(verbose) {
		    isarja_sarjaksi(isarja, pit, sarja);
		    printf("%s yhtenevä\n", sarja);
		}
		trexa = powi(3, pit); // jotta otetaan heti seuraava
		continue; }

	    unsigned kohta=0;
	    int lasku=0;
	    do {
		siirto(&kuutio, isarja[kohta*2], 0, isarja[kohta*2+1]);
		kohta = (kohta+1) % pit;
		lasku++;
	    } while(!onkoRatkaistu(&kuutio));
	    käytyjä++;

	    if(luku_listalle(lasku, tied.lista)) {
		puts("epäonnistui");
		return NULL; }
	    if(verbose) {
		isarja_sarjaksi(isarja, pit, sarja);
		printf("%s\t%i\n", sarja, lasku);
	    }
	}
	tied.tulostfun(&tied, käytyjä);
	/* Jos jokin muu säie on valmis, annetaan sille tästä osa, ellei jokin muu säie ehdi ensin. */
	for(int i=0; i<töitä; i++)
	    if(valmis[i] == 1 && !anna_työtä(&tied, sexa, i))
		break;
    }

ulos:
    free(kuutio.sivut);
    free(*kuutio.indeksit);
    return NULL;
}
#undef PIT_SARJA
#undef PIT_ISARJA

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

uint64 korjaa_sexa(uint64 sexa, int pit) {
    while(sexa_ei_kelpaa(sexa, pit)) sexa++;
    return sexa;
}

void seuraava_sarja(int pit, uint64 *sexa, uint64 *trexa) {
    if(++*trexa < powi(3, pit))
	return;
    *trexa = 0;
    *sexa = korjaa_sexa(++*sexa, pit);
}
