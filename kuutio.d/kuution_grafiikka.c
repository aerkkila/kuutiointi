#include <SDL2/SDL.h>
#include <stdarg.h>
#include <math.h>
#include <sys/time.h>
#include "kuutio.h"
#include "kuution_grafiikka.h"

char ohjelman_nimi[] = "Kuutio";
int ikkuna_x = 300;
int ikkuna_y = 300;
int ikkuna_w = 500;
int ikkuna_h = 500;
kuva_t kuva;
float kaantoaika;
float kaantoaika0 = 0.2;

int minKoordInd(koordf *ktit, int akseli, int pit);
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit);
#define PI 3.14159265358979
SDL_Texture* alusta[2];

koordf pyöräytä(koordf xyz, koordf kulmat) {
    float x = xyz.a[0], y = xyz.a[1], z = xyz.a[2];
    float x1,y1,z1;
    /*x-pyöräytys*/
    y1 = y*cosf(kulmat.a[0]) - z*sinf(kulmat.a[0]);
    z1 = y*sinf(kulmat.a[0]) + z*cosf(kulmat.a[0]);
    y = y1; z = z1;
    /*y-pyöräytys*/
    x1 = x*cosf(kulmat.a[1]) + z*sinf(kulmat.a[1]);
    z1 = -x*sinf(kulmat.a[1]) + z*cosf(kulmat.a[1]);
    x = x1; z = z1;
    /*z-pyöräytys*/
    x1 = x*cosf(kulmat.a[2]) - y*sinf(kulmat.a[2]);
    y1 = x*sinf(kulmat.a[2]) + y*cosf(kulmat.a[2]);
    x = x1; y = y1;
    return (koordf){{x,y,z}};
}
#define k(i) (koord.a[i])
#define u(i) (aks.a[i])
#define co (cosf(kulma))
#define si (sinf(kulma))
koordf yleispyöräytys(koordf koord, koordf aks, float kulma) {
    kulma = -kulma; // Rotaatiomatriisi pyörittää vastapäivään, halutaan myötäpäivään.
    float x,y,z;
    x = (k(0) * (co + u(0)*u(0)*(1-co)) +
	 k(1) * (u(0)*u(1)*(1-co) - u(2)*si) +
	 k(2) * (u(0)*u(2)*(1-co) + u(1)*si));
  
    y = (k(0) * (u(1)*u(0)*(1-co) + u(2)*si) +
	 k(1) * (co + u(1)*u(1)*(1-co)) +	
	 k(2) * (u(1)*u(2)*(1-co) - u(0)*si));

    z = (k(0) * (u(2)*u(0)*(1-co) - u(1)*si) +
	 k(1) * (u(2)*u(1)*(1-co) + u(0)*si) +
	 k(2) * (co + u(2)*u(2)*(1-co)));
    return (koordf){{x,y,z}};
}
#undef k
#undef u
#undef co
#undef si

int luo_kuva() {
    kuva.ikkuna = SDL_CreateWindow(ohjelman_nimi, ikkuna_x, ikkuna_y, ikkuna_w, ikkuna_h, SDL_WINDOW_RESIZABLE);
    kuva.rend = SDL_CreateRenderer(kuva.ikkuna, -1, SDL_RENDERER_TARGETTEXTURE);
    if(!kuva.rend) return 1;
    koordf asento = {{PI/6, -PI/6, 0}};
    kuva.kannat[0] = pyöräytä((koordf){{1,0,0}}, asento);
    kuva.kannat[1] = pyöräytä((koordf){{0,1,0}}, asento);
    kuva.kannat[2] = pyöräytä((koordf){{0,0,1}}, asento);
    kuva.ruudut = malloc(6*4*kuutio.N2*sizeof(koordf));
    kuva.xRes = ikkuna_w;
    kuva.yRes = ikkuna_h;
    kuva.mustaOsuus = 0.05;
    kuva.paivita = 1;
    kuva.korostus = -1;
    //kuva.siivukorostus = (int3){{-1, 
    kuva.ruutuKorostus = (int3){{-1, kuutio.N/2, kuutio.N/2}};
    kuva.varit[_u] = VARI(255,255,255); //valkoinen
    kuva.varit[_f] = VARI(0,  220,  0); //vihreä
    kuva.varit[_r] = VARI(255,0  ,0  ); //punainen
    kuva.varit[_d] = VARI(255,255,0  ); //keltainen
    kuva.varit[_b] = VARI(0,  0,  255); //sininen
    kuva.varit[_l] = VARI(220,120,0  ); //oranssi
    kuva.korostusVari = VARI(80, 233, 166);
    kuva.resKuut = (ikkuna_h < ikkuna_w)? ikkuna_h/sqrt(3.0)/2 : ikkuna_w/sqrt(3.0);
    /*jos mustaa on alle 1 ruutu, laitetaan 1 ruutu, jos osuus on silloin alle puolet*/
    if(kuva.resKuut/kuutio.N*kuva.mustaOsuus < 1)
	if(1/(kuva.resKuut/kuutio.N) < 0.5)
	    kuva.mustaOsuus = 1/(kuva.resKuut/kuutio.N);
    kuva.sij0 = (ikkuna_h < ikkuna_w)? (ikkuna_h-kuva.resKuut)/2: (ikkuna_w-kuva.resKuut)/2;
    return 0;
}

void paivita() {
    kuva.paivita = 0;
    SDL_SetRenderDrawColor(kuva.rend, 0, 0, 0, 255);
    SDL_RenderClear(kuva.rend);
    piirrä_kuvaksi();
    if(kuva.korostus >= 0)
	korosta_tahko(kuva.korostus);
    if(kuva.ruutuKorostus.a[0] >= 0)
	korosta_ruutu(kuva.ruudut+RUUTUINT3(kuva.ruutuKorostus), 3);
    SDL_RenderPresent(kuva.rend);
}

float vektpituus(koordf a) {
    return sqrt(pow(a.a[0],2) + pow(a.a[1],2));
}

float pistetulo(koordf a, koordf b) {
    return a.a[0]*b.a[0] + a.a[1]*b.a[1];
}

/*alueella, jos kierrosluku ≠ 0 eli ääriviivat kiertävät pisteen*/
int piste_alueella(float x, float y, int n, ...) {
    va_list argl;
    va_start(argl, n);
    float kl = 0;
    koordf sv1, sv2, a1, a2;
    koordf p = (koordf){{x, y, 0}};
    a1 = va_arg(argl, koordf);
    for(int i=1; i<=n; i++) {
	if(i==n)
	    va_start(argl,n); //ympäri asti eli viimeisenä ensimmäinen
	a2 = va_arg(argl, koordf);
	sv1 = suuntavektori(&p, &a1);
	sv2 = suuntavektori(&p, &a2);
	kl += (acosf( pistetulo(sv1,sv2) / (vektpituus(sv1)*vektpituus(sv2)) ) * \
	       SIGN(ristitulo_z(sv1,sv2)));
	a1 = a2;
    }
    return round(kl);
}

int mikä_tahko(int x, int y) {
    int tahko;
    for(tahko=0; tahko<6; tahko++) {
	/*näkyykö tahko*/
	if(ristitulo_z(suuntavektori(kuva.ruudut+RUUTU(tahko,0,0),
				     kuva.ruudut+RUUTU(tahko,0,kuutio.N-1)),
		       suuntavektori(kuva.ruudut+RUUTU(tahko,0,0),
				     kuva.ruudut+RUUTU(tahko,kuutio.N-1,0))) <= 0)
	    continue;
	/*näkyy, mutta onko oikea*/
	if(piste_alueella((float)x, (float)(-y), 4,			
			  kuva.ruudut[RUUTU(tahko,0,0)],
			  kuva.ruudut[RUUTU(tahko,kuutio.N-1,0)+1],
			  kuva.ruudut[RUUTU(tahko,kuutio.N-1,kuutio.N-1)+2],
			  kuva.ruudut[RUUTU(tahko,0,kuutio.N-1)+3]))
	    return tahko;
    }
    return -1;
}

int3 mikä_ruutu(int x, int y) {
    int tahko = mikä_tahko(x,y);
    if(tahko < 0)
	goto ei_löytynyt;
    float xf = x, yf = -y;
    float rMaks = 1.41421*kuva.resKuut/kuutio.N; // ruudun nurkan osuessa kauemmas hylätään heti
    for(int i=0; i<kuutio.N; i++)
	for(int j=0; j<kuutio.N; j++) {
	    koordf* tr = kuva.ruudut+RUUTU(tahko,i,j);
	    if(fabs(tr->a[0]-xf) > rMaks || fabs(tr->a[1]-yf) > rMaks)
		continue;
	    if(piste_alueella(xf, yf, 4, tr[0], tr[1], tr[2], tr[3]))
		return (int3){{tahko,i,j}};
	}
ei_löytynyt:
    return (int3){{-1,-1,-1}};
}

double hetkiNyt() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + t.tv_usec*1e-6;
}

static void animoi(int tahko, int kaista, int määrä) {
    static double viimeKaantohetki = -1.0;
    struct timeval hetki;
    gettimeofday(&hetki, NULL);
    double hetkiNyt = hetki.tv_sec + hetki.tv_usec*1e-6;
    double erotus = hetkiNyt - viimeKaantohetki;
    if(erotus < kaantoaika0)
	kaantoaika = erotus;
    else
	kaantoaika = kaantoaika0;
    koordf akseli = kuva.kannat[tahko%3];
    if(tahko>=3)
	for(int i=0; i<3; i++)
	    akseli.a[i] *= -1;
    kääntöanimaatio(tahko, kaista, akseli, määrä>2 ? -1 : määrä, kaantoaika);
    tee_ruutujen_koordtit();
    kuva.paivita = 1;
    gettimeofday(&hetki, NULL);
    viimeKaantohetki = hetki.tv_sec + hetki.tv_usec*1e-6;
}

static void __attribute__((unused)) siirto_(int tahko, int kaista, int määrä) {
    animoi(tahko, kaista, määrä);
    siirto(&kuutio, tahko, kaista, määrä);
    kuva.paivita = 1;
    kuutio.ratkaistu = onkoRatkaistu(&kuutio);
#ifndef __EI_SEKUNTIKELLOA__
    if(viimeViesti == ipcTarkastelu) {
	ipc->viesti = ipcAloita;
	viimeViesti = ipcAloita;
    }
    else if(viimeViesti == ipcAloita && kuutio.ratkaistu) {
	ipc->viesti = ipcLopeta;
	viimeViesti = ipcLopeta;
    }
#endif
}

#define TEE_RUUTU kuva.ruudut[RUUTU(tahko,i,j)+nurkka] = ruudun_nurkka(tahko, i, j, nurkka);
void tee_ruutujen_koordtit() {
    for(int tahko=0; tahko<6; tahko++)
	for(int i=0; i<kuutio.N; i++)
	    for(int j=0; j<kuutio.N; j++)
		for(int nurkka=0; nurkka<4; nurkka++)
		    TEE_RUUTU;
}
#undef TEE_RUUTU

void piirrä_kuvaksi() {
    for(int tahko=0; tahko<6; tahko++)
	for(int i=0; i<kuutio.N; i++)
	    for(int j=0; j<kuutio.N; j++) {
		vari vari = kuva.varit[(int)kuutio.sivut[SIVU(kuutio.N,tahko,i,j)]];
		aseta_vari(vari);
#define A(n) (kuva.ruudut+RUUTU(tahko,i,j)+n)
		if(ristitulo_z(suuntavektori(A(0), A(3)), suuntavektori(A(0), A(1))) > 0)
		    piirrä_suunnikas(kuva.ruudut+RUUTU(tahko, i, j));
#undef A
	    }
}

#define A(n) kuva.ruudut+RUUTUINT3(rtu)+n
void _piirrä_kaistoja(int tahko, int kaistaraja) {
    int3 rtu;
    for(int i=-kaistaraja; i<kuutio.N+kaistaraja; i++)
	for(int j=-kaistaraja; j<kuutio.N+kaistaraja; j++) {
	    rtu = hae_ruutu(kuutio.N, tahko, i, j);
	    if(rtu.a[0] < 0)
		continue;
	    vari vari = kuva.varit[(int)kuutio.sivut[SIVUINT3(kuutio.N,rtu)]];
	    aseta_vari(vari);
	    if(ristitulo_z(suuntavektori(A(0), A(3)), suuntavektori(A(0), A(1))) > 0)
		piirrä_suunnikas(kuva.ruudut+RUUTUINT3(rtu));
	}
}
#undef A

koordf ruudun_nurkka(int tahko, int iRuutu, int jRuutu, int nurkkaInd) {
    koordf nurkka, nurkka0; // nurkka0 on nurkan sijainti kuution omassa koordinaatistossa
    float res = kuva.resKuut/2;
    float x,y;
    /* haetaan oikea nurkka ruudusta (vasen ylä, vasen ala yms) */
    switch(nurkkaInd) {
    case 0: x = 0; y = 0; break;
    case 1: x = 1; y = 0; break;
    case 2: x = 1; y = 1; break;
    case 3: x = 0; y = 1; break;
    default:
	fprintf(stderr, "Virhe (tee_nurkan_koordtit): nurkkaInd = %x", nurkkaInd);
	return (koordf){{NAN, NAN, NAN}};
    }
    /* siirretään oikeaan ruutuun */
    float pala = kuva.resKuut/kuutio.N;
    float musta = pala*kuva.mustaOsuus/2;
    x = pala * (iRuutu + x) + (x? -1: 1)*musta;
    y = pala * (jRuutu + y) + (y? -1: 1)*musta;

    nurkka0.a[tahko%3]             = tahko>=3? -res: res;
    nurkka0.a[s_sivut[tahko][1]%3] = s_sivut[tahko][1]>=3 ? res-x : x-res;
    nurkka0.a[s_sivut[tahko][2]%3] = s_sivut[tahko][2]>=3 ? res-y : y-res;

    /* muunnos luonnolliseen koordinaatistoon:
       uusi_i = vanha_xyz * xyz:n vaikutus i:hin */
    for(int i=0; i<3; i++)
	nurkka.a[i] = (nurkka0.a[0] * kuva.kannat[0].a[i] +
		       nurkka0.a[1] * kuva.kannat[1].a[i] +
		       nurkka0.a[2] * kuva.kannat[2].a[i]);
  
    nurkka.a[0] = nurkka.a[0] + res + kuva.sij0;
    nurkka.a[1] = nurkka.a[1] - res - kuva.sij0;
    return nurkka;
}

void piirrä_suunnikas(koordf* ktit0) {
    koordf ktit[4];
    jarjestaKoord(ktit, ktit0, 0,  4); // nurkat x:n mukaan järjestettynä
    int xEro = (ktit[1].a[0] - ktit[0].a[0]) >= 0.5; // ei pystysuora

    float *yUla, *yAla;
    float kulmakerr1, kulmakerr2, y1, y2;
    if(xEro) {
	/* 0-nurkasta lähtevän kahden sivun kulmakertoimet */
	kulmakerr2 = ((ktit[2].a[1]-ktit[0].a[1]) / (ktit[2].a[0]-ktit[0].a[0]));
	kulmakerr1 = ((ktit[1].a[1]-ktit[0].a[1]) / (ktit[1].a[0]-ktit[0].a[0]));
	/* alempi ja ylempi y-koordinaatti ovat alkupisteessä samat ja eriytyvät oikealle mentäessä */
	y1 = -ktit[0].a[1];
	y2 = -ktit[0].a[1];
	if(kulmakerr1 < kulmakerr2) {
	    yAla = &y1;
	    yUla = &y2;
	}
	else {
	    yAla = &y2;
	    yUla = &y1;
	}
    }
    else { // On kaksi samaa kulmakerrointa. Katsottakoon näistä ylempi.
	int lisa0 = ktit[1].a[1] > ktit[0].a[1];
	int lisa1 = ktit[3].a[1] > ktit[2].a[1];
	kulmakerr1 = ((ktit[2+lisa1].a[1]-ktit[0+lisa0].a[1]) / (ktit[2+lisa1].a[0]-ktit[0+lisa0].a[0]));
	kulmakerr2 = kulmakerr1;
	y1 = -ktit[lisa0].a[1];
	yUla = &y1;
	y2 = -ktit[!lisa0].a[1];
	yAla = &y2;
    }
    float muisti=0;
    /* ylä- tai alapuolen kulmakerroin vaihtuu,
       kun saavutetaan kys. puolen nurkka */
    for(int patka=(xEro)? 0:2; patka<3; patka++) {
	int xraja = (int)(ktit[patka+1].a[0]);
	for(int i=ktit[patka*xEro].a[0]; i<xraja; i++) {
	    for(int j=*yUla; j<*yAla; j++)
		SDL_RenderDrawPoint(kuva.rend, i, j);
	    y1 -= kulmakerr1;
	    y2 -= kulmakerr2;
	}
	if(patka==0) {
	    muisti = kulmakerr1;
	    kulmakerr1 = (ktit[1].a[1]-ktit[3].a[1]) / (ktit[1].a[0]-ktit[3].a[0]);
	    y1 = -ktit[1].a[1];
	}
	else {
	    kulmakerr2 = muisti; // suunnikkaassa sama kulmakerroin toistuu
	    y2 = -ktit[2].a[1];
	}
    }
}

int minKoordInd(koordf *ktit, int akseli, int pit) {
    int minInd = 0;
    float min = ktit[0].a[akseli];
    for(int i=1; i<pit; i++)
	if(ktit[i].a[akseli] < min) {
	    min = ktit[i].a[akseli];
	    minInd = i;
	}
    return minInd;
}

/*luo uuden, jos ret == NULL*/
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit) {
    if(!ret)
	ret = malloc(pit*sizeof(koordf));
    koordf* juoksu = ret;
    koordf apu;
    for(int i=0; i<pit; i++)
	ret[i] = ktit[i];
    for(int i=0; i<pit; i++) {
	int ind = minKoordInd(juoksu, akseli, pit-i);
	apu = *juoksu;
	*juoksu = juoksu[ind];
	juoksu[ind] = apu;
	juoksu++;
    }
    return ret;
}

int korosta_tahko(int tahko) {
    if(tahko < 0 || tahko > 5)
	return 0;
    const int paksuus = 15;
    aseta_vari(kuva.korostusVari);
    piirrä_viiva(kuva.ruudut+RUUTU(tahko, 0, 0),
		 kuva.ruudut+RUUTU(tahko, kuutio.N-1, 0)+1,
		 3, paksuus);
    piirrä_viiva(kuva.ruudut+RUUTU(tahko, kuutio.N-1, 0)+1,
		 kuva.ruudut+RUUTU(tahko, kuutio.N-1, kuutio.N-1)+2,
		 3, paksuus);
    piirrä_viiva(kuva.ruudut+RUUTU(tahko, 0, kuutio.N-1)+3,
		 kuva.ruudut+RUUTU(tahko, kuutio.N-1, kuutio.N-1)+2,
		 3, paksuus);
    piirrä_viiva(kuva.ruudut+RUUTU(tahko, 0, 0),
		 kuva.ruudut+RUUTU(tahko, 0, kuutio.N-1)+3,
		 3, paksuus);
    return 1;
}

void korosta_ruutu(void* ktit, int onko2vai3) {
    const int paksuus = 10;
    aseta_vari(kuva.korostusVari);
    if(onko2vai3 == 2) {
	koordf2* k = ktit;
	piirrä_viiva(k, k+1, onko2vai3, paksuus);
	piirrä_viiva(k+1, k+2, onko2vai3, paksuus);
	piirrä_viiva(k+2, k+3, onko2vai3, paksuus);
	piirrä_viiva(k+3, k, onko2vai3, paksuus);
    }
    else {
	koordf* k = ktit;
	piirrä_viiva(k, k+1, onko2vai3, paksuus);
	piirrä_viiva(k+1, k+2, onko2vai3, paksuus);
	piirrä_viiva(k+2, k+3, onko2vai3, paksuus);
	piirrä_viiva(k+3, k, onko2vai3, paksuus);
    }
}

/*jos k2 == NULL, k1, on taulukko, jossa on molemmat*/
void piirrä_viiva(void* karg1, void* karg2, int onko2vai3, int paksuus) {
    int h2 = paksuus/2;
    koordf2 k1, k2;
    if(onko2vai3 == 2) {
	k1 = *(koordf2*)karg1;
	if(karg2)
	    k2 = *(koordf2*)karg2;
	else
	    k2 = (((koordf2*)karg1))[1];
    } else {
	k1 = (koordf2){{((koordf*)karg1)[0].a[0], ((koordf*)karg1)[0].a[1]}};
	if(karg2)
	    k2 = (koordf2){{((koordf*)karg2)[0].a[0], ((koordf*)karg2)[0].a[1]}};
	else
	    k2 = (koordf2){{((koordf*)karg1)[1].a[0], ((koordf*)karg1)[1].a[1]}};
    }
    /*kasvava x-koordinaatti*/
    if(k2.a[0] < k1.a[0])
	VAIHDA(k2, k1, koordf2);
    float yEro = k2.a[1] - k1.a[1];
    float xEro = k2.a[0] - k1.a[0];
    if(xEro < 1) {
	int iAlku = k1.a[0], jAlku = k1.a[1];
	int yEroi = yEro;
	if(yEroi > 0)
	    for(int j=0; j<yEroi; j++)
		for(int i=-h2; i<=h2; i++)
		    SDL_RenderDrawPoint(kuva.rend, i+iAlku, -(j+jAlku));
	else
	    for(int j=0; j>yEroi; j--)
		for(int i=-h2; i<=h2; i++)
		    SDL_RenderDrawPoint(kuva.rend, i+iAlku, -(j+jAlku));
	return;
    }
    if(fabs(yEro) < 1) {
	int jAlku = k1.a[1], iAlku = k1.a[0];
	int xEroi = xEro;
	if(xEroi > 0)
	    for(int i=0; i<xEroi; i++)
		for(int j=-h2; j<=h2; j++)
		    SDL_RenderDrawPoint(kuva.rend, iAlku+i, -(jAlku+j));
	else
	    for(int i=0; i>xEroi; i--)
		for(int j=-h2; j<=h2; j++)
		    SDL_RenderDrawPoint(kuva.rend, iAlku+i, -(jAlku+j));
	return;
    }
    float kulma = yEro / xEro;
    double kulmaT = -1/kulma;
    if(kulma >= 0) {
	int jNyt, jSeur, xEroi=xEro, jLoppu=k2.a[1];
	float iAlku, jAlku, jAlkuLoppu, jAlkuSeur, jAlkuNyt;
	iAlku = k1.a[0]; jAlku = k1.a[1];
	/*piirretään ohuita viivoja vierekkäin kunnes saavutetaan haluttu paksuus*/
	iAlku -= cos(atan(kulmaT))*h2;
	jAlku -= sin(atan(kulmaT))*h2; //kasvaa
	jAlkuLoppu = jAlku + (k1.a[1]-jAlku)*2; //loppu < alku

	/*ohuitten viivojen silmukka*/
	while(jAlku >= jAlkuLoppu) {
	    jAlkuSeur = jAlku + kulmaT; //seuraava on pienempi
	    /*jAlku voi muuttua alle 1:n, joten tähän silmukkaan otetaan toinen muuttuja*/
	    jAlkuNyt = jAlku;
	    /*tätä silmukkaa tarvitaan, jos kulmaT > 1; monta y-alkua samalla x-alulla*/
	    while(jAlkuNyt > jAlkuSeur && jAlkuNyt >= jAlkuLoppu) {
		for(int i=0; i<=xEroi; i++) {
		    jNyt = jAlkuNyt + i*kulma;
		    jSeur = jAlkuNyt + (i+1)*kulma;
		    while(jNyt <= jSeur && jNyt <= jLoppu)
			SDL_RenderDrawPoint(kuva.rend, i+iAlku, -jNyt++);
		}
		jAlkuNyt--; //kulmaT on negatiivinen
	    }
	    jAlku += kulmaT;
	    iAlku++;
	}
	return;
    }
    /*kulma < 0*/
    int jNyt, jSeur, xEroi=xEro, jLoppu=k2.a[1];
    float iAlku, jAlku, jAlkuLoppu, jAlkuSeur, jAlkuNyt;
    iAlku = k1.a[0]; jAlku = k1.a[1];
    /*piirretään ohuita viivoja vierekkäin kunnes saavutetaan haluttu paksuus*/
    iAlku -= cos(atan(kulmaT))*h2;
    jAlku -= sin(atan(kulmaT))*h2; //vähenee
    jAlkuLoppu = jAlku + (k1.a[1]-jAlku)*2; //loppu > alku
  
    while(jAlku <= jAlkuLoppu) {
	jAlkuSeur = jAlku + kulmaT; //seuraava on suurempi
	jAlkuNyt = jAlku;
	while(jAlkuNyt < jAlkuSeur && jAlkuNyt <= jAlkuLoppu) {
	    for(int i=0; i<=xEroi; i++) {
		jNyt = jAlkuNyt + i*kulma;
		jSeur = jAlkuNyt + (i+1)*kulma;
		while(jNyt >= jSeur && jNyt >= jLoppu) //kulma on negatiivinen
		    SDL_RenderDrawPoint(kuva.rend, i+iAlku, -jNyt--);
	    }
	    jAlkuNyt++; //kulmaT on positiivinen
	}
	jAlku += kulmaT;
	iAlku++;
    }
    return;
}

#define A (rtu[n])
static void _kääntöanim_pyöräytä(koordf akseli, float askel, int tahko, int i, int j) {
    float normitussiirto = kuva.resKuut/2 + kuva.sij0;
    int3 paikka;
    paikka = hae_ruutu(kuutio.N, tahko,i,j);
    if(paikka.a[0] < 0)
	return;
    koordf* rtu = kuva.ruudut+RUUTU(paikka.a[0],paikka.a[1],paikka.a[2]);
    for(int n=0; n<4; n++) { //n tarkoittaa ruudun jokaisen nurkan koordinaattia
	A = (koordf){{A.a[0]-normitussiirto, A.a[1]+normitussiirto, A.a[2]}}; //origo keskikohdaksi
	A = yleispyöräytys(A, akseli, askel);
	A = (koordf){{A.a[0]+normitussiirto, A.a[1]-normitussiirto, A.a[2]}}; //takaisin origosta
    }
    vari vari = VARIINT3(paikka);
    aseta_vari(vari);
    /* Näkymättömätkin piti kääntää, koska niistä voi tulla näkyviä myöhemmin käännön aikana. */
    if(ristitulo_z(suuntavektori(rtu+0, rtu+3), suuntavektori(rtu+0, rtu+1)) > 0)
	piirrä_suunnikas(rtu);
}
#undef A

void kääntöanimaatio(int tahko, int kaista, koordf akseli, double määrä, double aika) {
    const double spf = 1/30.0; // 1/fps
    double kokoKulma = PI/2*määrä;
    double kulmaNyt = 0.0;
    struct timeval hetki;
    gettimeofday(&hetki, NULL);
    double alku = hetki.tv_sec + hetki.tv_usec*1.0e-6;
    double loppu = alku + aika;
    double kului = 0;

    /*Monen kaistan samanaikainen pyöritys merkitään kaistan negatiivisuutena.*/

    /*Paikallaan pysyvä osa piirretään alussa toiseen tekstuuriin.
      Alle tuleva osa piirretään alustaan, jossa alfa-kanava on 255, jolloin ei tarvitse pyyhkiä ikkunaa
      Vain liikkuva osa piirretään aina uudestaan*/
    koordf* rtu = kuva.ruudut+RUUTU(tahko, 0, 0);
    int vastap_alla = ristitulo_z(suuntavektori(rtu+0, rtu+3), suuntavektori(rtu+0, rtu+1)) > 0; //onko vastapäinen alla
    int abskaista = kaista >= 0 ? kaista : -kaista;

    /* Alle tuleva alusta. */
    SDL_SetRenderTarget(kuva.rend, alusta[1]);
    SDL_SetRenderDrawColor(kuva.rend,0,0,0,255); // 1. tulee alle eli alfa = 255
    SDL_RenderClear(kuva.rend);
    /* Jos on liikkumaton osa, joka tulee alle, piirretään se.
       Piirrä_kaistoja-funktiossa ensimmäinen kaista on 1 ja pelkkä sivu on 0. */
    if (vastap_alla && (abskaista != kuutio.N-1))
	_piirrä_kaistoja((tahko+3)%6, kuutio.N-1-abskaista);
    else if (!vastap_alla && kaista > 0)
	_piirrä_kaistoja(tahko, kaista); // tämä on alla, joten piirretään aina tähän asti

    /* Päälle tuleva alusta. */
    SDL_SetRenderTarget(kuva.rend, alusta[0]);
    SDL_SetRenderDrawColor(kuva.rend,0,0,0,0); // 0. tulee päälle
    SDL_SetTextureBlendMode(alusta[0], SDL_BLENDMODE_BLEND); // varmistetaan alfa-kanava
    SDL_RenderClear(kuva.rend);
    /* Mahdollinen päälle tuleva liikkumaton osa. */
    if (vastap_alla && kaista > 0) // tämä tahko on päällä ja ei käännetä päällimmäistä kaistaa
	_piirrä_kaistoja(tahko, kaista); // aina tähän asti
    else if (!vastap_alla && abskaista < kuutio.N-1)
	_piirrä_kaistoja((tahko+3)%6, kuutio.N-1-abskaista);

    SDL_SetRenderTarget(kuva.rend, NULL);

    while (alku+kului/2 < loppu) {
	float askel = (kokoKulma - kulmaNyt) * spf / (loppu-alku);
	if(fabs(kulmaNyt+askel) > fabs(kokoKulma))
	    break;
	kulmaNyt += askel;

	/* Alusta[1] tulee alle. Sitten piirretään kuva ja lopuksi laitetaan alusta[0] päälle. */
	SDL_RenderCopy(kuva.rend, alusta[1], NULL, NULL);

	int kaista0, kaista1;
	if (kaista < 0)
	    kaista0 = 0, kaista1 = -kaista;
	else
	    kaista0 = kaista, kaista1 = kaista+1;

	for(int kaistanyt=kaista0; kaistanyt<kaista1; kaistanyt++) {
	    /* j-suunnassa molemmat i:n puolet */
	    for(int j=0; j<kuutio.N; j++) {
		_kääntöanim_pyöräytä(akseli, askel, tahko, -kaistanyt-1, j);
		_kääntöanim_pyöräytä(akseli, askel, tahko, kuutio.N+kaistanyt, j);
	    }
	    /* i-suunnassa molemmat j:n puolet */
	    for(int i=0; i<kuutio.N; i++) {
		_kääntöanim_pyöräytä(akseli, askel, tahko, i, -kaistanyt-1);
		_kääntöanim_pyöräytä(akseli, askel, tahko, i, kuutio.N+kaistanyt);
	    }
	    /* Tarvittaessa käännetään vielä sivu. */
	    int aputahko = tahko;
	    if (kaistanyt == kuutio.N-1) {
		aputahko = (tahko+3) % 6;
		goto sivun_kääntö;
	    }
	    if (kaistanyt == 0)
sivun_kääntö:
		for(int j=0; j<kuutio.N; j++)
		    for(int i=0; i<kuutio.N; i++)
			_kääntöanim_pyöräytä(akseli, askel, aputahko, i, j);
	}
	/*pysähdys*/
	gettimeofday(&hetki, NULL);
	double nyt = hetki.tv_sec + hetki.tv_usec*1.0e-6;
	kului = nyt - alku;
	if(kului < spf)
	    SDL_Delay((unsigned)((spf - kului)*1000));

	/*näyttämiseen kuluva aika lasketaan uuden kuvan tekemiseen*/
	gettimeofday(&hetki, NULL);
	alku = hetki.tv_sec + hetki.tv_usec*1.0e-6;
	SDL_RenderCopy(kuva.rend, alusta[0], NULL, NULL);
	SDL_RenderPresent(kuva.rend);
    }
}
