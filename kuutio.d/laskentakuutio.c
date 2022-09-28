/*
  Lasketaan monellako siirrolla pääsee takaisin alkuun toistamalla jotain yhdistelmää.
 
  1. siirto on aina r[], koska kuutiota kääntämällä muut siirrot voidaan muuntaa siksi.
  2. siirto on aina l[] tai u[], koska kuutiota kääntämällä muut voidaan muuttaa niiksi.

  Seuraavia ei ole vielä toteutettu:
  Yleisesti sanotaan esimerkiksi, että 1. siirto on x-akselin ympäri positiiviseen suuntaan positiiviselta puolelta
  ja 2. on y-akselin ympäri positiiviseen suuntaan positiiviselta puolelta.
  1. siirto määrittelee x-akselin, mutta ei suuntaa, jos tehdään perään toiselta puolelta eri suuntaan, esim R L.
  1. siirto määrittelee myös suunnan, jos tehdään R L'. Molemmat ovat samaan suuntaan, joten myötäpäivä saadaan määriteltyä.
  1. siirto määrittelee suunnan silloinkin, jos se on R2, jota ei seuraa L2.
  Tämä kiinnittää x-akselin positiivisen puolen ja siten myös suunnan.
  2. siirto määrittelee väkisin y-akselin (R L yms. laskettiin yhdeksi siirroksi)
  Esim. R2 L2 U' -sarjan jälkeen y2 on mahdollinen, koska R2 L2 kiinnittää x-akselin suunnatta ja U' kiinnittää y-akselin suuntineen
  R2 L2 U' D -sarjan jälkeen myös x2 tai z2 on mahdollinen, koska kaikki suunnat ovat kiinnittymättä
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "kuutio.c"

static const char* tahkot = "RUFLDB";
static const char* suunnat = " '2";
static const int isuunnat[] = {1,3,2};

long korjaa_sexa(long, int);
void sarjantekofunktio(int pit, long *sexa, long unsigned *trexa);
void* laskenta(void* vp);

long powi(long a, long unsigned n);
void tulosta_sarjana(long, long, int);

typedef struct {
    long sexa;
    long raja;
    int pit;
    int id;
} säikeen_tiedot;

/** merkitään siirtotahkot kuusikantaisilla sexaluvuilla
    esim. 0135 olisi R U L B
    lisättäessä 1 saadaan 0140 eli R U D R

    Vastaavasti suunnat merkitään kolmikantaisilla trexaluvuilla.

    Kannan muunnos tehdään tällä makrolla.
    Kun #luku muutetaan #kanta-kantaiseksi, mikä on vähiten merkitsevästä alkaen #i. numero.
    Jakojäännös noukkii i+1 viimeistä lukua, esim (kanta=10,i=1). 1215 % 10^2 -> 15.
    Jakolasku noukkii i:nnen luvun: 15 / 10^1 -> 1 */
#define NLUKU(luku, i, kanta) ((luku) % powi(kanta,i+1) / powi(kanta,i))

long powi(long a, long unsigned n) { // tämä toimii ajassa O(log(n))
    long r=1;
alku:
    if(!n) return r;
    if(n%2) r *= a;
    n /= 2;
    a *= a;
    goto alku;
}

int main(int argc, char** argv) {
    /*komentoriviargumentit*/
    int maxpit_l = 4;
    int minpit_l = 2;
    int töitä = 1;
    int apu;
    for(int i=0; i<argc-1; i++)
	if(!strcmp(argv[i], "--pit") || !strcmp(argv[i], "-n")) {
	    apu = sscanf(argv[i+1], "%i-%i", &maxpit_l, &minpit_l);
	    switch(apu) {
	    case 0: puts("Ei luettu pituuksia"); break;
	    case 2: VAIHDA_XOR(minpit_l, maxpit_l); break;
	    }
	}
	else if(!strcmp(argv[i], "--töitä") || !strcmp(argv[i], "-j")) {
	    if(!(sscanf(argv[i+1], "%i", &töitä)))
		puts("Ei luettu töitten määrää");
	}
    pthread_t säikeet[töitä];

#ifndef DEBUG
    for(int pit=minpit_l; pit<=maxpit_l; pit++) {
#else
	int pit = maxpit_l;
#endif
	long sexa1 = powi(6, pit-1); // Lopetetaan, kun ensimmäinen siirto olisi U, jolloin kaikki R ... on käyty.
	long pätkä = sexa1/töitä;
	säikeen_tiedot t[töitä];
	for(int i=0; i<töitä; i++) {
	    long raja0 = pätkä*i;
	    long raja1 = i==töitä-1 ? sexa1 : pätkä*(i+1);
	    t[i] = (säikeen_tiedot){.sexa=raja0, .raja=raja1, .pit=pit, .id=i};
	    pthread_create(säikeet+i, NULL, laskenta, t+i);
	}
	for(int i=0; i<töitä; i++)
	    pthread_join(säikeet[i], NULL);
	/*Alustetaan uusi tiedosto*/
	char apu[25];
	char snimi[22];
	sprintf(apu, "sarjat%i.csv", pit);
	FILE *ulos = fopen(apu, "w");
	/*Liitetään siihen säikeitten tekemät tiedostot*/
	char c;
	for(int i=0; i<töitä; i++) {
	    sprintf(snimi, "tmp%i.csv", i);
	    FILE *f = fopen(snimi, "r");
	    if(!f)
		continue;
	    while((c=fgetc(f)) > 0)
		fputc(c, ulos);
	    fclose(f);
#ifndef DEBUG
	    sprintf(apu, "rm %s", snimi);
	    system(apu);
#endif
	}
	fclose(ulos);
#ifndef DEBUG
    }
#endif
}

void tulosta_sarjana(long sexa, long trexa, int pit) {
    char sarja[32] = {0};
    for(int i=0; i<pit; i++) {
	sarja[(pit-i-1)*3] = tahkot[NLUKU(sexa, i, 6)];
	sarja[(pit-i-1)*3+1] = suunnat[NLUKU(trexa, i, 3)];
	sarja[(pit-i-1)*3+2] = ' ';
    }
    puts(sarja);
}

#define PIT_SARJA (pit*3)
#define PIT_ISARJA (pit*2*sizeof(int))
void* laskenta(void* vp) {
    kuutio_t kuutio;
    luo_kuutio(&kuutio, 3);
    säikeen_tiedot tied = *(säikeen_tiedot*)vp;
    int pit = tied.pit;
    char* sarja = malloc(PIT_SARJA+1);
    int* isarja = malloc(PIT_ISARJA);
    char* sarja0 = malloc(PIT_SARJA+1);
    int* isarja0 = malloc(PIT_ISARJA);
    memset(sarja,suunnat[0],PIT_SARJA+1);
    for(int i=0; i<pit*2; i++)
	isarja[i] = isuunnat[0];
    sarja[PIT_SARJA] = '\0';

    char nimi[12];
    sprintf(nimi, "tmp%i.csv", tied.id);
    FILE *f = fopen(nimi, "w");
    long unsigned trexa = -2;
    while(1) {
	sarjantekofunktio(pit, &tied.sexa, &trexa);
	if(tied.sexa >= tied.raja) break;
	unsigned kohta=0;
	int lasku=0;
	/*sarja kirjalliseen ja lukumuotoon*/
	for(int i=0; i<pit; i++) {
	    int ind = NLUKU(tied.sexa, i, 6);
	    sarja[(pit-i-1)*3] = tahkot[ind];
	    isarja[(pit-i-1)*2] = ind;
	    ind = NLUKU(trexa, i, 3);
	    sarja[(pit-i-1)*3+1] = suunnat[ind];
	    isarja[(pit-i-1)*2+1] = isuunnat[ind];
	}
	do {
	    siirto(&kuutio, isarja[kohta*2], 0, isarja[kohta*2+1]);
	    kohta = (kohta+1) % pit;
	    lasku++;
	} while(!onkoRatkaistu(&kuutio));
	fprintf(f, "%s\t%i\n", sarja,lasku);
    }
    fclose(f);
    free(sarja); free(isarja);
    free(sarja0); free(isarja0);
    free(kuutio.sivut);
    free(*kuutio.indeksit);
    return NULL;
}
#undef PIT_SARJA
#undef PIT_ISARJA

/* Sama sama tahko ei saa esiintyä kahdesti peräkkäin eikä sama akseli kolmesti. */
int sexa_ei_kelpaa(long sexa, int pit) {
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

long korjaa_sexa(long sexa, int pit) {
    while(sexa_ei_kelpaa(sexa, pit)) sexa++;
    return sexa;
}

void sarjantekofunktio(int pit, long *sexa, long unsigned *trexa) {
#ifndef DEBUG // suuntia ei käsitellä virheenjäljityksessä
    if(++*trexa < powi(3, pit)) return;
#endif
    *trexa = 0;
    *sexa = korjaa_sexa(++*sexa, pit);
}
