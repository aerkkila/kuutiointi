#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "../muistin_jako.h"

extern int viimeViesti;
void lue_siirrot_skellosta(shm_tietue*);
void lue_siirrot(const char* sarja);

int kaistaksi(char c) {
    int kaista = 0;
    if (c == 'x' || c == 'y' || c == 'z')
	kaista = -(kuutio.N - 1);
    else if ('a' <= c && c <= 'z')
	kaista = -kuutio.N/2;
    return kaista;
}

/* Isoilla kuutioilla kaista merkitään numerolla. */
int kaistaksi_iso(char c) {
    return c - '0';
}

/*Jos kuution koko saavuttaa 20, kaista voi olla suurempi kuin yksinumeroinen luku
  Tämä lukee kaistan ja kasvattaa indeksiä sopivan määrän*/
int kaistaksi_valtava(const char* str, int* id) {
    int kaista;
    if(sscanf(str + *id, "%i", &kaista) != 1)
	kaista = 0;
    do
	++*id;
    while('0' <= str[*id] && str[*id] <= '9');
    return kaista;
}

int tahkoksi(char c) {
    if (c == 'y')
	c = 'U';
    else if (c == 'x')
	c = 'R';
    else if (c == 'z')
	c = 'F';
    else if('a' <= c && c <= 'z')
	c -= 'a' - 'A';
    for(int i=0; i<6; i++)
	if(tahkojärjestys[i] == c)
	    return i;
    return -1;
}

int määräksi(char c) {
    switch(c) {
	case ' ':  return 1;
	case '2':  return 2;
	case '\'': return 3;
	default:   return -1;
    }
}

void lue_siirrot(const char* sarja) {
    int tahko=0,kaista=0,määrä=0,i=0;
    if(kuutio.N < 6)
	while(sarja[i]) {
	    kaista = kaistaksi(sarja[i]);
	    tahko  = tahkoksi(sarja[i++]);
	    määrä  = määräksi(sarja[i++]);
	    for(int j=0; j<=kaista; j++)
		siirto(&kuutio, tahko, j, määrä);
	    while(sarja[i] == ' ')
		i++;
	}
    else if(kuutio.N < 20)
	while(sarja[i]) {
	    kaista = kaistaksi_iso(sarja[i++]);
	    tahko  = tahkoksi(sarja[i++]);
	    määrä  = määräksi(sarja[i++]);
	    for(int j=0; j<=kaista; j++)
		siirto(&kuutio, tahko, j, määrä);
	    while(sarja[i] == ' ')
		i++;
	}
    else
	while(sarja[i]) {
	    kaista = kaistaksi_valtava(sarja, &i);
	    tahko  = tahkoksi(sarja[i++]);
	    määrä  = määräksi(sarja[i++]);
	    for(int j=0; j<=kaista; j++)
		siirto(&kuutio, tahko, j, määrä);
	    while(sarja[i] == ' ')
		i++;
	}
}

/*Ajanottosovellus laittaa sekoituksen jaettuun muistiin.
  Tämä funktio lukee siirrot jaetusta muistista ja kääntää kuutiota niitten mukaan*/
void lue_siirrot_skellosta(shm_tietue* ipc) {
    float aikaraja = 1.0;
    float nukkumaaika = 0.01e6; //µs
    float aika = 0;
    ipc->viesti = ipcAnna_sekoitus;
    viimeViesti = ipcAnna_sekoitus;
    /*ipc->viesti asetetaan nollaksi, kun valmista*/
    while(ipc->viesti && aika < aikaraja) {
	usleep(nukkumaaika);
	aika += nukkumaaika;
    }
    if(ipc->viesti) {
	ipc->viesti = 0;
	printf("Ei luettu siirtoja\n");
	return;
    }
    lue_siirrot(ipc->data);
}
