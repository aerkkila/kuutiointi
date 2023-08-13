#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <err.h>
#include <time.h>

#include "kuutio.c"
#include "kuution_grafiikka.c"

kuutio_t kuutio;

#include "lue_siirrot.c"

int* sijainnit;
char* sarjat;
char* selitteet;
char* peilautuuko;
int* iselitteet;
int sarjoja, isarja = 0, peilikuva = 0;

void lue_sarjat(const char* tiednimi) {
    FILE* f = fopen(tiednimi, "r");
    if (!f)
	err(1, "fopen %s", tiednimi);
    struct stat buf;
    if (fstat(fileno(f), &buf) < 0)
	err(2, "fstat");
    /* Jokainen peilattava sarja on ">>>"-alkuisella rivillä
       ja peilautumaton ">>="-alkuisella. */
    int tunniste_peil = ('>'<<2*8) + ('>'<<1*8) + ('>'<<0*8);
    int tunniste_pton = ('>'>>0*8) + ('>'<<1*8) + ('='<<2*8);
    int ind = 0, indseli = 0;
    sarjoja = 0;
    sarjat	= malloc(buf.st_size);
    sijainnit	= malloc(buf.st_size / 20 * sizeof(int));
    iselitteet	= malloc(buf.st_size / 20 * sizeof(int));
    peilautuuko	= malloc(buf.st_size / 20);
    selitteet	= malloc(buf.st_size);
    int selisij = 0;
    while(!feof(f)) {
	int yrite = 0;
	fread(&yrite, 1, 3, f);
	if (yrite != tunniste_peil && yrite != tunniste_pton) {
	    memcpy(selitteet+indseli, &yrite, 3);
	    if (selitteet[indseli] != '#') {
		indseli += 3;
		while((selitteet[indseli++] = fgetc(f)) != '\n' && !feof(f));
	    }
	    else
		while(fgetc(f) != '\n' && !feof(f));
	}
	else {
	    char c;
	    peilautuuko[sarjoja] = yrite == tunniste_peil;
	    sijainnit[sarjoja++] = ind;
	    selitteet[indseli++] = '\0';
	    iselitteet[sarjoja-1] = selisij;
	    selisij = indseli;
	    while((c = fgetc(f))!='\n' && !feof(f))
		if (c == '[')
		    while(fgetc(f) != ']' && !feof(f));
		else if (c == '(' || c == ')')
		    ;
		else
		    sarjat[ind++] = c;
	    sarjat[ind++] = '\0';
	}
    }
    fclose(f);
    sijainnit = realloc(sijainnit, sarjoja*sizeof(int));
    peilautuuko =  realloc(peilautuuko, sarjoja);
    sarjat = realloc(sarjat, ind);
}

void sarja_takaperin(const char* sarja) {
    int i = strlen(sarja)-1;
    int määrä, tahko, kaista;
    while (i>=0) {
	while(i>0 && sarja[i] <= ' ')
	    i--;
	if (('a' <= sarja[i] && sarja[i] <= 'z') ||
		('A' <= sarja[i] && sarja[i] <= 'Z'))
	    määrä = 1;
	else if (sarja[i] == '\'' && i>0 && sarja[i-1] == '2') { // 2' voi lukea sormitusten takia
	    i -= 2;
	    määrä = 2;
	}
	else
	    määrä = määräksi(sarja[i--]);
	if (i<0)
	    return;
	tahko  = tahkoksi(sarja[i]);
	kaista = kaistaksi(sarja[i--]);
	if (peilikuva) {
	    if (tahko % 3 == 0)
		tahko = (tahko+3) % 6;
	    /* Peilikuva ja takaperuus kumoavat toisensa, joten suuntaa ei muuteta. */
	}
	else
	    määrä = määrä * 3 % 4; // takaperin
		
	siirto(&kuutio, tahko, kaista, määrä);
    }
}

static void anim_täällä(int tahko, int kaista, int määrä) {
    koordf akseli = kuva.kannat[tahko%3];
    if(tahko>=3)
	for(int i=0; i<3; i++)
	    akseli.a[i] *= -1;
    double aika = 0.3;
    if (määrä != 2)
	määrä -= 2;
    else
	aika *= 1.5;
    kääntöanimaatio(tahko, kaista, akseli, määrä, aika);
    tee_ruutujen_koordtit();
    kuva.paivita = 1;
}

void tee_sarja(const char* sarja) {
    int i = 0, tahko, määrä, kaista;
    while(sarja[i]) {
	while(sarja[i] <= ' ' && sarja[i])
	    i++;
	if (!sarja[i])
	    return;
	tahko = tahkoksi(sarja[i]);
	kaista = kaistaksi(sarja[i++]);
	määrä = sarja[i] ? määräksi(sarja[i]) : 1;
	if (sarja[i])
	    while (sarja[++i] == '\''); // yksi i++ ei riitä, jos on 2'
	if (peilikuva) {
	    if (tahko % 3 == 0)
		tahko = (tahko + 3) % 6;
	    if (määrä > 0)
		määrä = määrä * 3 % 4;
	}
	anim_täällä(tahko, kaista, määrä);
	siirto(&kuutio, tahko, kaista, määrä);
	kuva.paivita = 1;
	paivita();
	usleep((int)(0.15e6));
    }
}

void alkuun() {
    for(int i=0; i<6; i++)
	memset(kuutio.sivut+i*kuutio.N2, i, kuutio.N2);
}

void seuraava_sarja() {
    isarja = rand() % sarjoja;
    alkuun();
    peilikuva = peilautuuko[isarja] && rand() % 2;
    sarja_takaperin(sarjat + sijainnit[isarja]);
    kuva.paivita = 1;
}

int main(int argc, char** argv) {
    srand(time(NULL));
    int N = 3;
    if (argc < 2)
	return 1;
    lue_sarjat(argv[1]);
    luo_kuutio(&kuutio, N);

    SDL_Init(SDL_INIT_VIDEO);

    if (luo_kuva())
	goto ulos;
    tee_ruutujen_koordtit();
    kaantoaika = kaantoaika0;
    for(int i=0; i<2; i++) {
	alusta[i] = SDL_CreateTexture(kuva.rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, kuva.xRes, kuva.yRes);
	SDL_SetTextureBlendMode(alusta[i], SDL_BLENDMODE_BLEND); //alustan kopioinnissa on alfa-kanava
	if (!alusta[i])
	    return 1;
    }
    SDL_SetRenderDrawBlendMode(kuva.rend, SDL_BLENDMODE_NONE); //muualla otetaan sellaisenaan

    SDL_Event tapaht;
silmukka:
    while (SDL_PollEvent(&tapaht)) {
	switch(tapaht.type) {
	    case SDL_QUIT:
		goto ulos;
	    case SDL_WINDOWEVENT:
		switch(tapaht.window.event) {
		    case SDL_WINDOWEVENT_RESIZED:
			;int koko1 = (tapaht.window.data1 < tapaht.window.data2)? tapaht.window.data1: tapaht.window.data2;
			kuva.xRes = tapaht.window.data1;
			kuva.yRes = tapaht.window.data2;
			for(int i=0; i<2; i++) {
			    SDL_DestroyTexture(alusta[i]);
			    if(!(alusta[i] = SDL_CreateTexture(kuva.rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, kuva.xRes, kuva.yRes)))
				printf("Ei tehty alustaa %i\n", i);
			    SDL_SetTextureBlendMode(alusta[i], SDL_BLENDMODE_BLEND);
			}
			kuva.resKuut = koko1/sqrt(3.0);
			if(kuva.resKuut/kuutio.N*kuva.mustaOsuus < 1)
			    if(1/(kuva.resKuut/kuutio.N) < 0.5)
				kuva.mustaOsuus = 1/(kuva.resKuut/kuutio.N);

			if((koko1-kuva.resKuut)/2 != kuva.sij0) {
			    kuva.sij0 = (koko1-kuva.resKuut)/2;
			    tee_ruutujen_koordtit();
			    kuva.paivita = 1;
			}
			break;
		}
		break;
	    case SDL_KEYDOWN:
		if (tapaht.key.keysym.sym == SDLK_SPACE)
		    seuraava_sarja();
		else if (tapaht.key.keysym.sym == SDLK_RETURN) {
		    puts(selitteet + iselitteet[isarja]);
		    if (peilikuva)
			printf("\033[91m");
		    puts(sarjat + sijainnit[isarja]);
		    if (peilikuva)
			printf("\033[0m");
		}
		else if (tapaht.key.keysym.sym == SDLK_UP)
		    tee_sarja(sarjat+sijainnit[isarja]);
		else if (tapaht.key.keysym.sym == SDLK_BACKSPACE) {
		    tee_sarja(sarjat+sijainnit[isarja]);
		    sarja_takaperin(sarjat+sijainnit[isarja]);
		    kuva.paivita = 1;
		}
		break;
	}
    }
    if(kuva.paivita)
	paivita();
    SDL_Delay(15);
    goto silmukka;

ulos:
    for(int i=0; i<2; i++)
	SDL_DestroyTexture(alusta[i]);
    SDL_DestroyRenderer(kuva.rend);
    SDL_DestroyWindow(kuva.ikkuna);
    free(kuva.ruudut);
    SDL_Quit();
    free(kuutio.sivut);
    free(*kuutio.indeksit);
    free(sijainnit);
    free(sarjat);
    free(selitteet);
    free(iselitteet);
    free(peilautuuko);
    return 0;
}
