#include <stdlib.h>
#include <string.h>
#include "kuutio.h"

/* akselit, näissä 0 korvataan 3:lla jotta saadaan etumerkki
   esim. oikealla (r) j liikuttaa negatiiviseen y-suuntaan (1.indeksi = y, ±2 = j)
   i liikuttaa negatiiviseen z-suuntaan (2. indeksi = z, ±1 = i)
   sijainti on positiivisella x-akselilla (0. indeksi = x, ±3 = sijainti) */
const int3 akst[6] = {			
    [_r] = {{ 3,-2,-1}},			
    [_l] = {{-3,-2, 1}},			
    [_u] = {{ 1, 3, 2}},			
    [_d] = {{ 1,-3,-2}},			
    [_f] = {{ 1,-2, 3}},			
    [_b] = {{-1,-2,-3}}
};
/* käänteinen yllä olevaan: luvut ovat tämä, i, j
   oikea on +x-akselilla (0. = tämä), i on -z-akseli (1. = i, ±2 = z) jne */
const int3 akst_tij[6] = {
    [_r] = {{ 3,-2,-1}},
    [_l] = {{-3, 2,-1}},
    [_u] = {{ 1, 3, 2}},
    [_d] = {{-1, 3,-2}},
    [_f] = {{ 2, 3,-1}},
    [_b] = {{-2,-3,-1}}
};

kuutio_t luo_kuutio(int N) {
    kuutio_t ktio;
    ktio.N = N;
    ktio.N2 = N*N;
    ktio.sivut = malloc(6*ktio.N2);
    ktio.ratkaistu = 1;
    for(int i=0; i<6; i++)
	memset(ktio.sivut+i*ktio.N2, i, ktio.N2);
    return ktio;
}

void siirto(kuutio_t* kuutp, int tahko, int siirtokaista, int maara) {
    if(maara == 0) return;
    int N = kuutp->N;
    if(siirtokaista < 0 || siirtokaista > N) return;
    if(siirtokaista == N && N > 1) {
	maara = (maara+2) % 4;
	tahko = (tahko+3) % 6;
	siirtokaista = 1;
    }
    /* siirretään kuin old-pochman-menetelmässä:
       vaihdetaan ensin sivut 0,1, sitten 0,2 jne */
    int iakseli = ABS(akst_tij[tahko].a[1]%3);
    int3 ruutu0 = hae_ruutu(N, tahko, 0, -siirtokaista);
    int IvaiJ = akst[ruutu0.a[0]].a[iakseli];
    for(int j=1; j<4; j++) {
	for(int i=0; i<N; i++) {
	    ruutu0 = hae_ruutu(N, tahko, i, -siirtokaista); // alkukohdaksi valitaan j-akselin alimeno
	    int etumerkki = SIGN(IvaiJ) * SIGN(akst[tahko].a[iakseli]);
	    int3 ruutu1 = hae_ruutu(N,
				    ruutu0.a[0],
				    ruutu0.a[1] + (2-ABS(IvaiJ)) * j*N * etumerkki,
				    ruutu0.a[2] + (ABS(IvaiJ)-1) * j*N * etumerkki);
	    VAIHDA(kuutp->sivut[SIVUINT3(N,ruutu0)], kuutp->sivut[SIVUINT3(N,ruutu1)], char);
	}
    }
    /* siivusiirrolle (slice move) kääntö on nyt suoritettu */
    if(siirtokaista > 1) {
	siirto(kuutp, tahko, siirtokaista, maara-1);
	return;
    }
    /* käännetään käännetty sivu */
    char* sivu = kuutp->sivut+tahko*N*N;
    char apu[N*N];
    memcpy(apu, sivu, N*N);

    /* nyt käännetään: */
#define arvo(taul,j,i) taul[(i)*N+(j)]
    for(int i=0; i<N; i++)
	for(int j=0; j<N; j++)
	    *sivu++ = arvo(apu,N-1-i,j);
#undef arvo
    siirto(kuutp, tahko, siirtokaista, maara-1);
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

int s_sivut[6][4] = {
    [_r] = {_u,_b},
    [_l] = {_u,_f},
    [_u] = {_b,_r},
    [_d] = {_f,_r},
    [_f] = {_u,_r},
    [_b] = {_u,_l},
};
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

int _ijmuunnos_negpos(int kuutio_N, int ij) { return ij + kuutio_N; }
int _ijmuunnos_pospos(int kuutio_N, int ij) { return kuutio_N*2-1 - ij; }
int _ijmuunnos_negneg(int kuutio_N, int ij) { return -ij - 1; }
int _ijmuunnos_posneg(int kuutio_N, int ij) { return ij - kuutio_N; }
int (*_ijmuunnos_[])(int,int) = {
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
