#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include "asetelma.h"
#include "tulokset.h"

float* flomituslajittele_jarj(float*, int*, int); //tämä on omassa tiedostossaan
int fmaxind(float*, int);
int fminind(float*, int);
float keskiarvo(float*, int);
float std(float* taul, int,  float ka, int otos);
void karstaulkot(float** ulos, float* sis, int pit, int n, int pp, int ps);
sakko_e hae_sakko(const char* s);

/*Tämä kutsuttakoon aina, kun tuloksia on muutettu.
  Tämä huolehtii myös järjestyslistan ajantasaisuudesta
  ja muualla sen oletetaan olevan ajan tasalla.*/
slista* tee_tiedot(int* avgind) {
  char apuc[600];
  tuhjenna_slista(tietoloput);
  float ka[ftulos->pit];
  int a=0, b=0, c=0;

  /*Avg5 ja Avg12*/
  float** kars = malloc(ftulos->pit*sizeof(float*));
  for(int i=0; i<ftulos->pit; i++)
    kars[i] = malloc((10)*sizeof(float)); //tätä kokoa voi joutua muuttamaan
  /**************************************************/
  for(int q=5; q<=12; q+=7) {
    if(ftulos->pit < q)
      sprintf(apuc, " = %.2f  (%.2f – %.2f)", NAN, NAN, NAN);
    else {
      karstaulkot(kars, ftulos->taul, ftulos->pit, q, 1, 1);
      for(int i=0; i<=ftulos->pit-q; i++)
	ka[i] = keskiarvo(kars[i], q-2);
      a = ftulos->pit-q;
      b = fminind(ka, ftulos->pit-q+1);
      c = fmaxind(ka, ftulos->pit-q+1);
      /*Tuloksiin lisätään 5 ms, jotta pyöristys menee oikein*/
      sprintf(apuc, " = %.99s  (%.99s – %.99s)",	\
	      float_kelloksi(apuc+200,ka[a]+0.005),	\
	      float_kelloksi(apuc+300,ka[b]+0.005),	\
	      float_kelloksi(apuc+400,ka[c]+0.005));
    }
    slistalle_kopioiden(tietoloput, apuc);
    if(ftulos->pit < q)
      sprintf(apuc, " = %.2f   %.2f   %.2f", NAN, NAN, NAN);
    else
      sprintf(apuc, " = %.2f   %.2f   %.2f",				\
	      std(kars[a],q-2,ka[a],0), std(kars[b],q-2,ka[b],0), std(kars[c],q-2,ka[c],0));
    slistalle_kopioiden(tietoloput, apuc);
    if(avgind) {
      avgind[0] = a;
      avgind[1] = b;
      avgind[2] = c;
      avgind += 3;
    }
  }
  for(int i=0; i<ftulos->pit; i++)
    free(kars[i]);
  free(kars);

  if(ftulos->pit <= 0) {
    sprintf(apuc, " = %.2f (σ = %.2f); %i", NAN, NAN, karsinta);
    slistalle_kopioiden(tietoloput, apuc);
    sprintf(apuc, " = %.2f (%i.)", NAN, -1);
    slistalle_kopioiden(tietoloput, apuc);
    return tietoloput;
  }
  /*järjestyslista*/
  fjarje = realloc(fjarje, ftulos->pit*ftulos->koko);
  jarjes = realloc(jarjes, ftulos->pit*sizeof(int));
  memcpy(fjarje, ftulos->taul, ftulos->pit*ftulos->koko);
  flomituslajittele_jarj(fjarje, jarjes, ftulos->pit);
  
  /*Keskiarvo*/
  int pois = ftulos->pit / karsinta + 1;
  int mukaan = ftulos->pit - pois*2;
  ka[0] = keskiarvo(fjarje+pois, mukaan);
  ka[1] = std(fjarje+pois, mukaan, ka[0], 1);
  sprintf(apuc, " = %s (σ = %.2f); %i", float_kelloksi(apuc+200,ka[0]+0.005), ka[1], karsinta);
  slistalle_kopioiden(tietoloput, apuc);

  /*Mediaani*/
  a = jarjes[(ftulos->pit-1)/2];
  *ka = ftulos->taul[a];
  if(!(ftulos->pit % 2)) {
    b = jarjes[(ftulos->pit-1)/2+1];
    *ka += ftulos->taul[b];
    *ka /= 2;
  }
  sprintf(apuc, " = %s (%i.)", float_kelloksi(apuc+200,*ka+0.005), a+1);
  slistalle_kopioiden(tietoloput, apuc);
  return tietoloput;
}

/*palauttaa listan, montako tulosta on kullakin kokonaissekunnilla
  esim. 15, 4, 16, 6, -1 tarkoittaisi 4 15:n sekunnin ja 6 16:n sekunnin tulosta
  -1 merkitsee loppua ja (int)inf << 0 eli aina loppuu negatiiviseen*/
int* eri_sekunnit(const flista* restrict ftul) {
  /*lasketaan ja alustetaan tarvittavien sekuntien määrä*/
  int n = 0;
  int vanha_sek = -1;
  for(int i=0; i<ftul->pit; i++)
    if( vanha_sek != (int)(fjarje[i]) ) {
      vanha_sek = (int)(fjarje[i]);
      n++;
    }
  int *erisek = calloc(n*2+1, sizeof(int));
  
  /*tehdään lista*/
  int sek = -1, i = -1;
  for(int j=0; j<ftulos->pit; j++) {
    int vanha = sek;
    if(vanha == (sek = (int)fjarje[j])) //sama sekunti kuin viime kerralla
      erisek[i*2+1]++;
    else {
      erisek[++i*2] = sek;
      erisek[i*2+1]++;
    }
  }
  erisek[n*2] = -1;
  return erisek;
}

/*laittaa yhden suurimman ja pienimmän sulkeisiin*/
void tee_lisatiedot(char** sektus, int alkuind, int n) {
  char apuc[700];
  if(alkuind+n > ftulos->pit)
    return;
  float *karstaul = malloc(n*sizeof(float));
  float* arr = ftulos->taul+alkuind;
  karstaulkot(&karstaul, arr, n, n, 1, 1);

  time_t aikatmp = thetki->taul[alkuind+n-1];
  struct tm *aika = localtime(&aikatmp);
  strftime(apuc+300, 200, "%A %d.%m.%Y klo %H.%M", aika);

  float ka = keskiarvo(karstaul, n-2);
  sprintf(apuc, "Avg%i: %s; σ = %.2f; %s",\
	  n, float_kelloksi(apuc+500,ka+0.005), std(karstaul, n-2, ka, 0), apuc+300);
  slistalle_kopioiden(lisatd, apuc);

  int ma = fmaxind(arr, n);
  int mi = fminind(arr, n);
  for(int i=0; i<n; i++) {
    if(i==ma || i==mi)
      sprintf(apuc, "(%i. %s)", i+alkuind+1, stulos->taul[alkuind+i]);
    else
      sprintf(apuc, " %i. %s", i+alkuind+1, stulos->taul[alkuind+i]);
    if(sektus)
      sprintf(apuc+strlen(apuc), "   %s", sektus[alkuind+i]);
    slistalle_kopioiden(lisatd, apuc);
  }
  free(karstaul);
}

void lisaa_listoille(const char* restrict kello, time_t hetki) {
  slistalle_kopioiden(stulos, kello);
  flistalle(ftulos, lue_kellosta(kello));
  ilistalle(thetki, hetki);
}

void poista_listoilta(int ind) {
  poista_slistalta(sektus, ind);
  poista_slistalta(stulos, ind);
  poista_listalta(ftulos, ind);
  poista_listalta(thetki, ind);
}

void poista_listoilta_viimeinen() {
  free(*VIIMEINEN(sektus));
  free(*VIIMEINEN(stulos));
  sektus->pit--; stulos->pit--; ftulos->pit--; thetki->pit--;
}
#include <stdio.h>
float lue_kellosta(const char* restrict s) {
  if(strstr(s, "Ø"))
    return INFINITY;
  float fsek=0;
  short min=0;
  if(!strstr(s, ":"))
    sscanf(s, "%f", &fsek);
  else
    sscanf(s, "%hi:%f", &min, &fsek);
  return min*60 + fsek;
}

char* float_kelloksi(char* kello, float f) {
  if(f+0.00001 < 60.0)
    sprintf(kello, "%.2f", f+0.00001);
  else if(isfinite(f)) {
    int sek = (int)(f+0.00001);
    char c = (char)( (f+0.00001 - sek) * 100 );
    sprintf(kello, "%i:%s%i,%s%hhi", sek/60,
	    (sek%60 < 10)? "0": "", sek%60,
	    (c<10)? "0": "", c);
  } else
    sprintf(kello, "Ø");
  return kello;
}

int lue_tiedosto(const char* tiednimi, char* rajaus) {
  FILE *f = fopen(tiednimi, "r");
  char apu[200];
  sprintf(apu, "%s%s", TULOSKANSIO, tiednimi);
  if( !f && !(f=fopen(apu, "r")) ) {
      fprintf(stderr, "Virhe, ei tiedostoa \"%s\"\n", tiednimi);
      return 1;
  }

  float faika;
  int i;
  char c;
  char* kello = kellool.teksti;
  setlocale(LC_NUMERIC, "C");
  int fpit0 = ftulos->pit;
  while(1) {
    while((c=fgetc(f)) < 0x21) //välien yli
      if(c == EOF)
	goto LUETTU;
    if(c == '#') //kommenttirivi
      while( (c=fgetc(f)) != '\n')
	if(c==EOF)
	  goto LUETTU;
    fseek(f, -1, SEEK_CUR);
    if(fscanf(f, "%f%i", &faika, &i) < 2) {
      fprintf(stderr, "Virhe, tiedostosta \"%s\" ei luettu arvoa\n", tiednimi);
      goto LUETTU;
    }
    flistalle(ftulos, faika);
    ilistalle(thetki, i);
  }
 LUETTU:
  fclose(f);
  setlocale(LC_NUMERIC, getenv("LANG"));
  for(int i=fpit0; i<ftulos->pit; i++)
    slistalle_kopioiden(stulos, float_kelloksi(kello, ftulos->taul[i]));
  /*rajataan*/
  if(strlen(rajaus)) {
    int alku, loppu; //loppu on 1., jota ei tule
    if( sscanf(rajaus, ":%i", &loppu) == 1) {
      alku = 0;
    } else {
      int skan = sscanf(rajaus, "%i:%i", &alku, &loppu);
      switch(skan) {
      case 0:
      case -1:
	fprintf(stderr, "Virheellinen rajausargumentti\n");
	break;
      case 1:
	loppu = ftulos->pit;
	break;
      }
    }
    if(alku < 0)
      alku += ftulos->pit;
    if(loppu < 0)
      loppu += ftulos->pit+1;
    if(loppu < alku)
      return 0;
    rajaa_lista(ftulos, alku, loppu);
    rajaa_slista(stulos, alku, loppu);
    rajaa_lista(thetki, alku, loppu);
  }
  return 0;
}

/*teksti voi olla kello tai turha*/
void muuta_sakko(char* teksti, int ind) {
  if(ind >= stulos->pit)
    return;
  char* tul = stulos->taul[ind];
  float* fp = ftulos->taul+ind;
  int min=0;
  sakko_e sakko = hae_sakko(tul);

  switch(sakko) {
  case ei:
    *fp += 2;
    break;
  case plus:
    *fp -= 2;
    break;
  case dnf:
    if(!strstr(tul, ":")) {
      sscanf(tul, "Ø(%f)", fp);
    } else {
      float fsek;
      sscanf(tul, "Ø(%i:%f)", &min, &fsek);
      *fp = min*60 + fsek;
    }
    break;
  }
  min = (int)(*fp+0.00001) / 60;

  sakko = (sakko+1) % 3;
  if(!min) {
    sprintf(teksti, "%s%.2f%s",					\
	    (sakko==dnf)? "Ø(" : "", *fp,			\
	    (sakko==ei)? "" : ( (sakko==plus)? "+" : ")" ));
  } else {
    float fsek = *fp+0.00001 - min*60;
    char muoto[30];
    if (fsek < 10)
      strcpy(muoto, "%s%i:0%.2f%s");
    else
      strcpy(muoto, "%s%i:%.2f%s");
    sprintf(teksti, muoto,					\
	    (sakko==dnf)? "Ø(" : "",				\
	    min, fsek,						\
	    (sakko==ei)? "" : ( (sakko==plus)? "+" : ")" ));
  }
  stulos->taul[ind] = realloc(stulos->taul[ind], strlen(teksti)+1);
  strcpy(stulos->taul[ind], teksti);
  
  if(sakko == dnf)
    *fp = INFINITY;
}

int tallenna(const char* restrict tiednimi) {
  if(ftulos->pit <= 0)
    return 1;
  int ret = 0;
  setlocale(LC_NUMERIC, "C");
  FILE *f = fopen(tiednimi, "r+");
  /*jos tiedostoa ei ollut, se tehdään, muuten haetaan oikea kohta*/
  if(!f) {
    if(!(f=fopen(tiednimi, "w")))
      return 2;
    ret = -1; //uusi tiedosto
    goto KIRJOITA;
  }
  
  /*kirjoitetaan jo olleen ajan päälle, jos on samalta hetkeltä tiedostossa*/
  time_t hetki = thetki->taul[0];
  time_t yrite = 0;
  char c = 0;
  char apuc[300];
  while(1) {
    fflush(f);
    unsigned long sij = ftell(f);
    int skan = fscanf(f, "%*f %li\n", &yrite);
    if(skan == EOF)
      goto KIRJOITA;
    if(!skan) {
      while( (c=fgetc(f)) < 0x21 && c && c != '\n' && c != EOF ); //välit ohi rivillä
    SWITCH:
      switch(c) {
      case '#': //kommenttirivi
	while((c=fgetc(f)) != '\n' && c != EOF);
	goto SWITCH;
      case '\n':
	continue;
      case EOF:
      case 0:
	goto KIRJOITA;
      default:
	sprintf(apuc, "Virhe: Luettiin virheellinen merkki: c = %hhx (hexa) '%c'", c, c);
	fprintf(stderr, "%s\n", apuc);
	tuhjenna_slista(lisatd);
	slistalle_kopioiden(lisatd, apuc);
	laitot |= lisatdlai;
	setlocale(LC_NUMERIC, getenv("LANG"));
	return 1;
      }
    } else {
      if (yrite >= hetki) {
	fseek(f, sij, SEEK_SET); //takaisin rivin alkuun, tämän päälle kirjoitetaan
	goto KIRJOITA;
      }
    }
  }
 KIRJOITA:
  for(int i=0; i<ftulos->pit; i++)
    fprintf(f, "%.2f\t%i\n", ftulos->taul[i], thetki->taul[i]);
  fclose(f);
  setlocale(LC_NUMERIC, getenv("LANG"));
  return ret;
}

inline int __attribute__((always_inline)) fmaxind(float* taul, int n) {
  int m = 0;
  for(int i=1; i<n; i++)
    if(taul[i] > taul[m])
      m = i;
  return m;
}
inline int __attribute__((always_inline)) fminind(float* taul, int n) {
  int m = 0;
  for(int i=1; i<n; i++)
    if(taul[i] < taul[m])
      m = i;
  return m;
}

float keskiarvo(float* taul, int n) {
  double ka = 0;
  for(int i=0; i<n; i++)
    ka += taul[i];
  return ka/n;
}

float std(float* taul, int n, float ka, int otos) { //otos on 0 tai 1
  double summa = 0;
  for(int i=0; i<n; i++)
    summa += (taul[i]-ka)*(taul[i]-ka);
  return sqrtf(summa/(n-otos));
}

/*Tekee listan *float[n-kars], jossa jokaisella listalla ovat ne luvut,
  jotka tulevat mukaan senhetkiseen liukuvaan keskiarvoon. Argumentit:
  ulostulo, lähde, sen pituus, montako mukaan, karsinta: pois pieniä, pois suuria*/
void karstaulkot(float** ulos, float* taul, int pit, int n, int pp, int ps) {
  if(pit<n)
    return;
  float apu[n];
  for(int i=0; i<=pit-n; i++) {
    for(int j=0; j<n; j++)
      apu[j] = taul[i+j];
    /*pienet ja suuret arvot pois*/
    int k=0;
    for(int j=0; j<pp; j++, k++) {
      int m = fminind(apu+k, n-k) + k;
      VAIHDA(apu[k], apu[m], float);
    }
    for(int j=0; j<ps; j++, k++) {
      int m = fmaxind(apu+k, n-k) + k;
      VAIHDA(apu[k], apu[m], float);
    }
    /*nyt listalle*/
    for(int j=0; k<n; k++, j++)
      ulos[i][j] = apu[k];
  }
  return;
}

sakko_e hae_sakko(const char* s) {
  if(strstr(s, "Ø"))
    return dnf;
  if(strstr(s, "+"))
    return plus;
  return ei;
}

float* flomituslajittele_jarj_yli1(float* l0, int* taul, int n);

/*lomituslajittelu ja lisäksi syntynyt järjestys kirjoitetaan taulukkoon taul*/
float* flomituslajittele_jarj(float* l0, int* taul, int n) {
  for(int i=0; i<n; i++)
    taul[i] = i;
  if(n >= 2)
    flomituslajittele_jarj_yli1(l0, taul, n);
  return l0;
}

/*tämä kutsutaan yllä olevasta funktiosta*/
float* flomituslajittele_jarj_yli1(float* l0, int* taul, int n) {
  if(n <= 3) {
    float* l1 = l0+1;
    float iarr[] = {*l0, *l1};
    int pienempi = iarr[1] < iarr[0];
    *l0 = iarr[pienempi];
    *l1 = iarr[!pienempi];
    int taularr[] = {taul[0], taul[1]};
    taul[0] = taularr[pienempi];
    taul[1] = taularr[!pienempi];
    if(n==2)
      return l0;
    float* l2 = l1+1;
    iarr[0] = *l1; iarr[1] = *l2;
    pienempi = iarr[1] < iarr[0];
    *l1 = iarr[pienempi];
    *l2 = iarr[!pienempi];
    taularr[0] = taul[1]; taularr[1] = taul[2];
    taul[1] = taularr[pienempi];
    taul[2] = taularr[!pienempi];

    iarr[0] = *l0; iarr[1] = *l1;
    pienempi = iarr[1] < iarr[0];
    *l0 = iarr[pienempi];
    *l1 = iarr[!pienempi];
    taularr[0] = taul[0]; taularr[1] = taul[1];
    taul[0] = taularr[pienempi];
    taul[1] = taularr[!pienempi];
    return l0;
  }
  
  /*puolikkaitten järjestäminen*/
  int raja = n/2; // n/2
  flomituslajittele_jarj_yli1(l0, taul, raja);
  flomituslajittele_jarj_yli1(l0+raja, taul+raja, n-raja);
  float* l[] = {l0, l0+raja};

  float* muisti = malloc(n*sizeof(float));
  int id=0; int pienempi;
  int* muistitaul = malloc(n*sizeof(int));
  for(int i=0; i<n; i++)
    muistitaul[i] = taul[i];
  int* t[] = {muistitaul, muistitaul+raja};
  int rajat[] = {raja, n-raja};

  /*lomitus*/
  do {
    pienempi = *l[1] < *l[0];
    muisti[id] = *l[pienempi]++;
    taul[id++] = *t[pienempi]++;
  } while(--rajat[pienempi]);
  /*kopioidaan loput*/
  for(int i=0; i<rajat[!pienempi]; i++) {
    muisti[id] = l[!pienempi][i];
    taul[id++] = t[!pienempi][i];
  }
  /*kopioidaan taulukko takaisin listaan*/ //voisiko vain vaihtaa osoittimet?
  for(int i=0; i<id; i++)
    l0[i] = muisti[i];
  
  free(muisti);
  free(muistitaul);

  return l0;
}
