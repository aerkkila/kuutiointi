#ifndef __luokittelupuu_h__
#define __luokittelupuu_h__
#include <string.h>
#include <stdlib.h>

typedef struct lpuu_lehti lpuu_lehti;
typedef struct {float f; int i;} lpuu_floatint;

#define Assert(...) if(!(__VA_ARGS__)) do{printf("Tiedosto %s: rivi %i: virhe\n", __FILE__, __LINE__); asm("int $3");}while(0);

struct lpuu_lehti {
    /* eteenpäin */
    int muuttuja;
    float arvo;
    short seur[2];

    /* luokittelu */
    char luokka;
    float gini;
    short edel;
};

typedef struct {
    int lehtiä;
    int kapasit;
    int luokkia;
    lpuu_lehti *lehdet;
} lpuu;

typedef struct {
    int pit1, pit2;
    float* data;
} lpuu_matriisi;

typedef struct {
    lpuu *puu;
    lpuu_matriisi *x;
    char *y, *maski;
} lpuu_A;

#define Xmat(x,j,i) ((x)->data[(j)*(x)->pit2 + (i)])

static void lpuu_lisäyslajittele(lpuu_A* kaikki, int sarake) {
    lpuu_matriisi *x = kaikki->x;
    char* y = kaikki->y;
    char* maski = kaikki->maski;
    
    float xm[x->pit2];
    char ym;
    for(int i=1; i<x->pit1; i++) {
	if(!maski[i]) continue;
	int ind = i;
	while(ind && (!maski[ind-1] || Xmat(x, ind-1, sarake) > Xmat(x, i, sarake))) ind--;

	memcpy(xm, &Xmat(x, i, 0), x->pit2*sizeof(float));
	memmove(&Xmat(x, ind+1, 0), &Xmat(x, ind, 0), x->pit2*(i-ind)*sizeof(float));
	memcpy(&Xmat(x, ind, 0), xm, x->pit2*sizeof(float));
	ym = y[i];
	memmove(y+ind+1, y+ind, i-ind);
	y[ind] = ym;
	ym = maski[i];
	memmove(maski+ind+1, maski+ind, i-ind);
	maski[ind] = ym;
    }
}

static void lpuu_luokittele(lpuu_A* kaikki, int ilehti) {
    int pit = kaikki->x->pit1;
    lpuu *puu = kaikki->puu;
    char* restrict y = kaikki->y;

    int luokassa[puu->luokkia];
    memset(luokassa, 0, puu->luokkia*sizeof(int));
    for(int i=0; i<pit; i++)
	if(kaikki->maski[i])
	    luokassa[(int)y[i]]++;
    int maxluok = 0;
    int summa = luokassa[0];
    for(int i=1; i<puu->luokkia; i++) {
	summa += luokassa[i];
	if(luokassa[i] > luokassa[maxluok])
	    maxluok = i;
    }
    kaikki->puu->lehdet[ilehti].luokka = maxluok;

    float gini = 0;
    for(int i=0; i<puu->luokkia; i++) {
	float osam = (float)luokassa[i]/summa;
	gini += osam * (1-osam);
    }
    kaikki->puu->lehdet[ilehti].gini = gini;
}

static void lpuu_hae_lehti(lpuu_A* kaikki, int ilehti_sis) {
    lpuu_lehti* lehti = kaikki->puu->lehdet + ilehti_sis;
    int ilehti_, ilehti=ilehti_sis, pit=kaikki->x->pit1, sarake;
    char *maski = kaikki->maski;
    lpuu_matriisi* x = kaikki->x;

    memset(maski, 1, pit);

    while(((ilehti_=lehti->edel) >= 0)) {
	lehti = kaikki->puu->lehdet + ilehti_;
	sarake = lehti->muuttuja;
	if(lehti->seur[1] == ilehti)
	    for(int i=0; i<pit; i++)
		maski[i] = maski[i] && Xmat(x, i, sarake) >= lehti->arvo;
	else
	    for(int i=0; i<pit; i++)
		maski[i] = maski[i] && Xmat(x, i, sarake) < lehti->arvo;
	ilehti = ilehti_;
    }
}

/* Hakee parhaan jakokohdan halutulle lehdelle kysytyllä muuttujalla.
   Tämä olettaa, ettei lehti ole vielä täysin tarkka eli lisäjako tehdään. */
static lpuu_floatint lpuu_jako_muuttujalla(lpuu_A* kaikki, int sarake, int ilehti) {
    if(kaikki->puu->kapasit < kaikki->puu->lehtiä+2)
	return (lpuu_floatint){.f=2, .i=-1};
    float paras_gini = 2;
    int paras_i = -1;
    int pit=kaikki->x->pit1;
    lpuu_lehti *leh0 = kaikki->puu->lehdet+ilehti, *leh[2];
    lpuu_matriisi* xmat = kaikki->x;

    lpuu_hae_lehti(kaikki, ilehti);
    lpuu_lisäyslajittele(kaikki, sarake);
    char maski0[kaikki->x->pit1]; // tallennetaan tämän lehden maski
    memcpy(maski0, kaikki->maski, kaikki->x->pit1);

    leh0->muuttuja = sarake;
    leh0->seur[0] = kaikki->puu->lehtiä++;
    leh0->seur[1] = kaikki->puu->lehtiä++;
    leh[0] = kaikki->puu->lehdet + leh0->seur[0];
    leh[1] = kaikki->puu->lehdet + leh0->seur[1];
    *leh[0] = *leh[1] = (lpuu_lehti){-1};
    leh[0]->edel = leh[1]->edel = ilehti;
    for(int i=0; i<2; i++)
	leh[0]->seur[i] = leh[1]->seur[i] = 0;

    /* Reunalta niin pitkästi kuin y on samaa ei kokeilla erikseen.
       Tämä vaikuttaa myös tuloksiin eikä ainoastaan optimoi. */
    int alku=1, loppu=pit-1;
    while(!maski0[alku]  || kaikki->y[alku -1]==kaikki->y[alku ]) ++alku;
    while(!maski0[loppu] || kaikki->y[loppu-1]==kaikki->y[loppu]) --loppu;
    Assert(alku <= loppu);

    for(int i=alku; i<=loppu; i++) {
	if(!maski0[i]) break; // lajittelu siirtää maskin ulkopuoliset loppuun
	leh0->arvo = (Xmat(xmat, i, sarake) + Xmat(xmat, i-1, sarake)) * 0.5;
	for(int j=0; j<2; j++) {
	    lpuu_hae_lehti (kaikki, leh0->seur[j]); // voisi optimoida käyttämällä leh0:n maskia
	    lpuu_luokittele(kaikki, leh0->seur[j]);
	}
	int suurempi = leh[1]->gini > leh[0]->gini;
	if(leh[suurempi]->gini < paras_gini) {
	    paras_gini = leh[suurempi]->gini;
	    paras_i = i;
	}
    }
    /* Otetaan käyttöön se jako, joka oli paras */
    Assert(paras_i >= 0);
    leh0->arvo = (Xmat(xmat, paras_i, sarake) + Xmat(xmat, paras_i-1, sarake)) * 0.5;
    for(int j=0; j<2; j++) {
	lpuu_hae_lehti (kaikki, leh0->seur[j]);
	lpuu_luokittele(kaikki, leh0->seur[j]);
    }
    return (lpuu_floatint){.f=paras_gini, .i=paras_i};
}

static void jaa_tämä_lehti(lpuu_A* kaikki, int ilehti) {
    lpuu_floatint uusi_jako = {.f=2};
    int paras_muutt = 0;
    /* yhden lisäjaon teko */
    for(int i=0; i<kaikki->x->pit2; i++) {
	lpuu_floatint fi = lpuu_jako_muuttujalla(kaikki, i, ilehti);
	kaikki->puu->lehtiä -= 2;
	if(fi.f < uusi_jako.f) {
	    uusi_jako = fi;
	    paras_muutt = i;
	}
    }
    lpuu_jako_muuttujalla(kaikki, paras_muutt, ilehti); // voisiko käyttää uusi_jako.i:tä
    for(int i=1; i<=2; i++)
	if(kaikki->puu->lehdet[kaikki->puu->lehtiä-i].gini > 0)
	    jaa_tämä_lehti(kaikki, kaikki->puu->lehtiä-i);
}

/* kutsuttavissa */
static void lpuu_tee_jako(lpuu_matriisi* xmat, char* yvec, lpuu* puu) {
    char* maski = malloc(xmat->pit1);
    memset(maski, 1, xmat->pit1);
    puu->lehtiä = 1; // ensimmäinen on aina olemassa
    puu->lehdet[0].edel = -1;

    lpuu_A kaikki = {.puu=puu, .x=xmat, .y=yvec, .maski=maski};
    lpuu_luokittele(&kaikki, 0);
    jaa_tämä_lehti(&kaikki, 0);
    free(maski);
}

/* kutsuttavissa */
static void vapauta_lpuu(lpuu* lp) {
    if(!lp) return;
    free(lp->lehdet);
    *lp = (lpuu){0};
    free(lp);
}

/*kutsuttavissa */
static void vapauta_matriisi(lpuu_matriisi* m) {
    if(!m) return;
    free(m->data);
    m->data = NULL;
    free(m);
}

/* kutsuttavissa */
static int lpuu_kerro_luokka(const lpuu* puu, const float* x, float suvaitsevuus) {
    lpuu_lehti *lehti = puu->lehdet;
    while(lehti->seur[0] && lehti->gini > suvaitsevuus)
	lehti = puu->lehdet + lehti->seur[x[lehti->muuttuja] > lehti->arvo];
    return lehti->luokka;
}

/* kutsuttavissa */
static lpuu* luokittelupuu(lpuu_matriisi* xmat, char* yvec, int luokkia) {
    /* Kopioidaan x ja y, jotta niitä on vapaus järjestää uudelleen. */
    size_t pit = xmat->pit1*xmat->pit2*sizeof(float);
    lpuu_matriisi x1 = *xmat;
    x1.data = malloc(pit);
    memcpy(x1.data, xmat->data, pit);

    char y1[xmat->pit1];
    memcpy(y1, yvec, xmat->pit1);

    lpuu *puu = calloc(1, sizeof(lpuu));
    puu->lehdet = malloc(256*sizeof(lpuu_lehti));
    puu->kapasit = 256;
    puu->luokkia = luokkia;
    lpuu_tee_jako(&x1, y1, puu);
    free(x1.data);
    return puu;
}

#undef Xmat
#endif // ifndef __luokittelupuu_h__
