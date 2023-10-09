#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "kuutio.h"

/* Mitkä neljä tahkoa kohdataan kierrettäessä mikin sivu myötäpäivään
   alkaen ylhäältä eli positiivisesta y-suunnasta.
   Tässä alustetaan käsin kaksi ensimmäistä sivua jolloin loputkin kaksi tunnetaan. */
int s_sivut[6][4] = {
    [_r] = {_u,_b},
    [_l] = {_u,_f},
    [_u] = {_b,_r},
    [_d] = {_f,_r},
    [_f] = {_u,_r},
    [_b] = {_u,_l},
};

/* Millä indeksillä (e1) mikin sivu löytyy minkäkin sivun (e0) ympäriltä s_sivut-matriisissa.
   Esim. s_sivut_arg[_f][_r] == 1
   ja s_sivut_arg[_f][_b] == -1 (ei löydy) */
int s_sivut_arg[6][6];

void alusta_matriisit() {
    int ind(int tahko, int tahko2) {
	for(int i=0; i<4; i++)
	    if(s_sivut[tahko][i] == tahko2)
		return i;
	return -1;
    }
    for(int i=0; i<6; i++)
	for(int j=0; j<2; j++)
	    s_sivut[i][j+2] = (s_sivut[i][j]+3) % 6;
    for(int i=0; i<6; i++)
	for(int j=0; j<6; j++)
	    s_sivut_arg[i][j] = ind(i,j);
}

void luo_kuution_indeksit(int* taul[3], int pit) {
    int pit1 = pit*pit*4;
    assert((*taul = malloc(pit1*3*sizeof(int))));
    for(int tahko=0; tahko<3; tahko++) {
	taul[tahko] = taul[0] + tahko*pit1;
	int ind=0;
	for(int kaista=0; kaista<pit; kaista++) {
	    for(int i=0; i<pit; i++) {
		int3 r0 = hae_ruutu(pit, tahko, i, -1-kaista);
		taul[tahko][ind++] = SIVUINT3(pit,r0);
	    }
	    for(int j=0; j<pit; j++) {
		int3 r0 = hae_ruutu(pit, tahko, pit+kaista, j);
		taul[tahko][ind++] = SIVUINT3(pit,r0);
	    }
	    for(int i=pit-1; i>=0; i--) {
		int3 r0 = hae_ruutu(pit, tahko, i, pit+kaista);
		taul[tahko][ind++] = SIVUINT3(pit,r0);
	    }
	    for(int j=pit-1; j>=0; j--) {
		int3 r0 = hae_ruutu(pit, tahko, -1-kaista, j);
		taul[tahko][ind++] = SIVUINT3(pit,r0);
	    }
	}
    }
}

void luo_kuutio(kuutio_t *ktio, int N) {
    alusta_matriisit();
    ktio->N = N;
    ktio->N2 = N*N;
    luo_kuution_indeksit(ktio->indeksit, ktio->N);
    int apupit = ktio->N2 > 4*ktio->N ? ktio->N2 : 4*ktio->N;
    ktio->sivut = malloc(6*ktio->N2 + apupit);
    ktio->apu = ktio->sivut + 6*ktio->N2;
    ktio->ratkaistu = 1;
    for(int i=0; i<6; i++)
	memset(ktio->sivut+i*ktio->N2, i, ktio->N2);
}

void _tahkon_pyöritys1(char* sivu, char* apu, int N) {
    for(int j=0; j<N; j++)
	for(int i=0; i<N; i++)
	    *sivu++ = apu[SIVU2(0, N, 0, N-1-i, j)];
}

void _tahkon_pyöritys2(char* sivu, char* apu, int N) {
    for(int j=0; j<N; j++)
	for(int i=0; i<N; i++)
	    *sivu++ = apu[SIVU2(0, N, 0, N-1-j, N-1-i)];
}

void _tahkon_pyöritys3(char* sivu, char* apu, int N) {
    for(int j=0; j<N; j++)
	for(int i=0; i<N; i++)
	    *sivu++ = apu[SIVU2(0, N, 0, i, N-1-j)];
}

void (*_tahkon_pyöritys[])(char*, char*, int) = {
    _tahkon_pyöritys1, _tahkon_pyöritys2, _tahkon_pyöritys3
};

void siirto(kuutio_t* ku, int tahko, int siirtokaista, int määrä) {
    if (!määrä)
	return;
    if(määrä < 0)
	määrä += 4;
    int N = ku->N;
    int tahko0 = tahko;
    if (siirtokaista < 0) { // monen kaistan pyöritys
	for(int i=0; i<=-siirtokaista; i++)
	    siirto(ku, tahko, i, määrä);
	return;
    }
    if(siirtokaista >= N) return;
    if(tahko >= 3) {
	tahko -= 3;
	siirtokaista = N - siirtokaista - 1;
	määrä = 4 - määrä;
    }
    /* Siivun pyöritys */
    int Nn = N*4;
    int *indptr = ku->indeksit[tahko] + Nn*siirtokaista;
    for(int i=0; i<Nn; i++)
	ku->apu[i] = ku->sivut[indptr[i]];
    Nn = N*määrä;
    int seisake = N*4 - Nn;
    for(int i=0; i<seisake; i++)
	ku->sivut[indptr[i+Nn]] = ku->apu[i];
    for(int i=0; i<Nn; i++)
	ku->sivut[indptr[i]] = ku->apu[i+seisake];

    /* Tahkon pyöritys */
    if(tahko0 >= 3) {
	siirtokaista = N - siirtokaista - 1;
	määrä = 4 - määrä;
    }
    if(siirtokaista == N-1) {
	tahko0 = (tahko0+3) % 6;
	määrä = 4 - määrä;
    }
    else if(siirtokaista != 0) return;
    char* sivu = ku->sivut + SIVU2(ku->N2, N, tahko0, 0, 0);
    memcpy(ku->apu, sivu, ku->N2);
    _tahkon_pyöritys[määrä-1](sivu, ku->apu, N);
}

int onkoRatkaistu(kuutio_t* kuutp) {
    int N2 = kuutp->N*kuutp->N;
    for(int sivu=0; sivu<6; sivu++) {
	char* restrict s = kuutp->sivut+sivu*N2;
	for(int i=1; i<N2; i++)
	    if(s[i] != *s)
		return 0;
    }
    return 1;
}

static int _ijmuunnos_negpos(int kuutio_N, int ij) { return ij + kuutio_N; }
static int _ijmuunnos_pospos(int kuutio_N, int ij) { return kuutio_N*2-1 - ij; }
static int _ijmuunnos_negneg(int kuutio_N, int ij) { return -ij - 1; }
static int _ijmuunnos_posneg(int kuutio_N, int ij) { return ij - kuutio_N; }
static int (*_ijmuunnos_[])(int,int) = {
    _ijmuunnos_pospos,
    _ijmuunnos_posneg,
    _ijmuunnos_negpos,
    _ijmuunnos_negneg,
};
int3 hae_ruutu(int kuutio_N, int tahko0, int i0, int j0) {
alku:
    int ji0[]={j0,i0}, ji1[2];
    int ylitys, ylittävä, neg0, neg1;
    int (*muunnosfun)(int,int);
    if(0);
    else if(j0<0)         { ylitys=0; ylittävä=0; neg0=1; }
    else if(j0>=kuutio_N) { ylitys=2; ylittävä=0; neg0=0; }
    else if(i0<0)	  { ylitys=3; ylittävä=1; neg0=1; }
    else if(i0>=kuutio_N) { ylitys=1; ylittävä=1; neg0=0; }
    else return (int3){{tahko0, i0, j0}};
    int tahko1 = s_sivut[tahko0][ylitys];

    int saapuva;
    switch(s_sivut_arg[tahko1][tahko0]) {
	case 0: saapuva=0; neg1=1; break;
	case 2: saapuva=0; neg1=0; break;
	case 3: saapuva=1; neg1=1; break;
	case 1: saapuva=1; neg1=0; break;
	default: exit(1);
    }
    muunnosfun = _ijmuunnos_[neg0*2+neg1];
    ji1[saapuva] = muunnosfun(kuutio_N, ji0[ylittävä]);

    /* toinen indeksi katsotaan, että pysyy saman tahkon vieressä */
    int sivullinen = s_sivut[tahko0][2-!ylittävä]; // positiivisesta i- tai j-suunnasta
    if(s_sivut_arg[tahko1][sivullinen] % 3)        // positiivinen uudellakin sivulla
	ji1[!saapuva] = ji0[!ylittävä];
    else
	ji1[!saapuva] = kuutio_N-1 - ji0[!ylittävä];

    i0 = ji1[1];
    j0 = ji1[0];
    tahko0 = tahko1;
    goto alku;
}
