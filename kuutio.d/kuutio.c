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
    if(maara == 0)
	return;
    int N = kuutp->N;
    if(siirtokaista < 0 || siirtokaista > N)
	return;
    else if(siirtokaista == N && N > 1) {
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

/* esim _u, 3, 0 (3x3x3-kuutio) --> _r, 2, 0:
   palauttaa oikean ruudun koordinaatit, kun yksi indeksi menee alunperin yli tahkolta */
int3 hae_ruutu(int kuutio_N, int tahko0, int i0, int j0) {
    int IvaiJ = (i0<0)? -1: (i0>=kuutio_N)? 1: (j0<0)? -2: (j0>=kuutio_N)? 2: 0;
    if(!IvaiJ)
	return (int3){{tahko0, i0, j0}};
  
    int aksTahko0;
    int3 ruutu0 = (int3){{tahko0, i0, j0}};
    aksTahko0 = ABS(tahko0%3);
    if((i0 < 0 || i0 >= kuutio_N) && (j0 < 0 || j0 >= kuutio_N))
	return (int3){{-1, -1, -1}};
    int ylimenoaks;
    for(ylimenoaks=0; ylimenoaks<3; ylimenoaks++)
	if(ABS(akst[tahko0].a[ylimenoaks]) == ABS(IvaiJ)) // akseli, jolta kys tahko menee yli
	    break;
    int tahko1;
    int haluttu = 3*SIGN(IvaiJ)*SIGN(akst[tahko0].a[ylimenoaks]); // menosuunta ja lukusuunta
    for(tahko1=0; tahko1<6; tahko1++)
	if(akst[tahko1].a[ylimenoaks] == haluttu) break;
    int ij1[2], ind;
    /* tullaanko alkuun vai loppuun eli kasvaako uusi indeksi vanhan tahkon sijainnin suuntaan
       myös, että tullaanko i- vai j-akselin suunnasta */
    int tulo = akst[tahko1].a[aksTahko0] * SIGN(akst[tahko0].a[aksTahko0]);
    ind = ABS(tulo)-1; // ±1->0->i, ±2->1->j
#define A ruutu0.a[ABS(IvaiJ)]
    int lisa = (A<0)? -A-1: A-kuutio_N;
    ij1[ind] = (tulo<0)? 0 + lisa : kuutio_N-1 - lisa ; // negat --> tullaan negatiiviselta suunnalta
#undef A
    ind = (ind+1)%2;
    /* toisen indeksin akseli on molempiin tahkoakseleihin kohtisuorassa */
    int akseli2 = 3-aksTahko0-ABS(ylimenoaks);
    ij1[ind] = (ABS(akst[tahko0].a[akseli2]) == 1)? i0: j0;
    /* vaihdetaan, jos ovat vastakkaissuuntaiset */
    if(akst[tahko0].a[akseli2] * akst[tahko1].a[akseli2] < 0)
	ij1[ind] = kuutio_N-1 - ij1[ind];
    return hae_ruutu(kuutio_N, tahko1, ij1[0], ij1[1]);
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
