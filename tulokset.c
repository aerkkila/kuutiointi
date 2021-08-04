#include <lista_math.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "asetelma.h"
#include "tulokset.h"

avgtulos avgn(flista* l, int n, int pois) {
  avgtulos r;
  flista *ml = _yalkuun(floatmovavg(_yalkuun(l), n-1, 0, pois, pois, -1));
  r.nyt = (ml)? ((flista*)_yloppuun(ml))->f : NAN;

  /*maks ja min*/
  floatint tmp = floatmax(ml, -1);
  r.max = tmp.a;
  r.maxind = tmp.b;
  tmp = floatmin(ml, -1);
  r.min = tmp.a;
  r.minind = tmp.b;
  _yrma(ml);
  return r;
}

double sigma(flista* fl, int n, int pois) {
  flista* l = _yalkuun(_fkopioi(fl, n));
  unsigned pit = _ylaske(l);
  if(pit != n)
    return NAN;
  flista* alku = l;
  float_lomitusjarj_pit(l, pit);
  for(int i=0; i<pois && l; i++)
    l = l->seur;
  double r = floatstd(l, n-(pois*2));
  _yrma(alku);
  return r;
}

/*palauttaa listan, montako tulosta on kullakin kokonaissekunnilla
  esim. 15, 4, 16, 6, -1 tarkoittaisi 4 15:n sekunnin ja 6 16:n sekunnin tulosta
  -1 merkitsee loppua
  tarvittaessa ia:an alustetaan lisää tilaa*/
int* eri_sekunnit(flista* jarj, int* ia, int iapit) {
  /*lasketaan ja alustetaan tarvittavien sekuntien määrä*/
  int n = 0;
  int vanha_sek = -1;
  if(!ia && jarj)
    ia = malloc(1);
  flista* jarj0 = jarj;
  while(jarj) {
    if(vanha_sek != (int)(jarj->f)) {
      vanha_sek = (int)(jarj->f);
      n++;
    }
    jarj = jarj->seur;
  }
  jarj = jarj0;
  if(iapit < n*2+1)
    ia = realloc(ia, (n*2+1)*sizeof(int));

  /*tehdään lista*/
  for(int i=0; i<n; i++) {
    int sek = (int)(jarj->f);
    ia[i*2] = sek;
    ia[i*2+1] = 0;
    while((int)(jarj->f) == sek) {
      ia[i*2+1]++;
      if( !(jarj = jarj->seur) ) {
	ia[(i+1)*2] = -1;
	return ia;
      }
    }
  }
  printf("\"eri_sekunnit\"-funktio ei loppunut asianmukaisesti\n");
  return ia;
}

/* Strtiedot-lista kirjoitetaan listan "tietoalut" viereen eli rakenne löytyy sieltä.
   Kys. lista taas luodaan merkkijonosta "tietoalkustr"*/
strlista* tee_tiedot(strlista* strtiedot, int* avgind) {
  flista* fl = ftulos;
  flista* fj = fjarj;
  avgtulos at;
  char *tmp;
  tmp = calloc(200, 1);
  _strpoista_kaikki(_yalkuun(strtiedot));
  strtiedot = NULL;

  /*Avg5*/
  at = avgn(fl, 5, 1);
  sprintf(tmp, " = %.2f  (%.2f – %.2f)", at.nyt, at.min, at.max);
  strtiedot = _strlisaa_kopioiden(strtiedot, tmp);
  double a, b, c;
  a = sigma(_ynouda(fl, -4), 5, 1);
  b = sigma(_ynouda(_yalkuun(fl), at.minind-4), 5, 1);
  c = sigma(_ynouda(_yalkuun(fl), at.maxind-4), 5, 1);
  sprintf(tmp, " = %.2lf   %.2lf   %.2lf", a, b, c);
  strtiedot = _strlisaa_kopioiden(strtiedot, tmp);
  if(avgind) {
    avgind[0] = _ylaske(_yalkuun(fl))-1;
    avgind[1] = at.minind;
    avgind[2] = at.maxind;
    avgind += 3;
  }

  /*Avg12*/
  at = avgn(fl, 12, 1);
  sprintf(tmp, " = %.2f  (%.2f – %.2f)", at.nyt, at.min, at.max);
  strtiedot = _strlisaa_kopioiden(strtiedot, tmp);
  a = sigma(_ynouda(fl, -11), 12, 1);
  b = sigma(_ynouda(_yalkuun(fl), at.minind-11), 12, 1);
  c = sigma(_ynouda(_yalkuun(fl), at.maxind-11), 12, 1);
  sprintf(tmp, " = %.2lf   %.2lf   %.2lf", a, b, c);
  strtiedot = _strlisaa_kopioiden(strtiedot, tmp);
  if(avgind) {
    avgind[0] = _ylaske(_yalkuun(fl))-1;
    avgind[1] = at.minind;
    avgind[2] = at.maxind;
  }

  /*Keskiarvo*/
  int pois = _ylaske_taakse(fl) / karsinta + 1;
  int mukaan = _ylaske_taakse(fl) - pois*2;
  sprintf(tmp, " = %.2lf (σ = %.2f); %i",				\
	  floatmean(_ynouda(fj, pois+1), mukaan), floatstd(_ynouda(fj, pois+1), mukaan), karsinta);
  strtiedot = _strlisaa_kopioiden(strtiedot, tmp);

  /*Mediaani*/
  floatint med = floatmed(_yalkuun(fl), -1);
  sprintf(tmp, " = %.2f (%i.)", med.a, med.b+1);
  strtiedot = _strlisaa_kopioiden(strtiedot, tmp);
  free(tmp);
  return strtiedot;
}

/*laittaa yhden suurimman ja pienimmän sulkeisiin*/
strlista* tee_lisatiedot(strlista* sektus, int alkuind, int n) {
  strlista* r = NULL;
  char tmp[150];
  char aikastr[100];
  strlista* sl = _ynouda(_yalkuun(strtulos), alkuind);
  flista* fl = _ynouda(_yalkuun(ftulos), alkuind);
  if(sektus) sektus = _ynouda(_yalkuun(sektus), alkuind);
  if(!sl)
    return NULL;

  /*tehdään aikastring*/
  time_t aika_t = ((ilista*)_ynouda(_yalkuun(tuloshetki), alkuind+n-1))->i;
  struct tm *aika = localtime(&aika_t);
  strftime(aikastr, 150, "%A %d.%m.%Y klo %H.%M", aika);

  sprintf(tmp, "Avg%i: %.2f; σ = %.2f; %s", n, floatavg(fl, 0, n-1, 1, 1), sigma(fl, n, 1), aikastr);
  r = _strlisaa_kopioiden(r, tmp);
  floatint max = floatmax(fl, n);
  floatint min = floatmin(fl, n);
  for(int i=0; i<n; i++) {
    if(i == max.b || i == min.b)
      sprintf(tmp, "(%i. %s)", i+1+alkuind, sl->str);
    else
      sprintf(tmp, " %i. %s", i+1+alkuind, sl->str);
    if(sektus) {
      sprintf(tmp, "%s   %s", tmp, sektus->str);
      sektus = sektus->seur;
    }
    r = _strlisaa_kopioiden(r, tmp);
    sl = sl->seur;
  }
  return _yalkuun(r);
}

/*mille paikalle f laitetaan listassa, joka on järjestetty pienimmästä suurimpaan*/
int hae_paikka(float f, flista* l) {
  if(!isfinite(f))
    return 0x0fffffff; //ei laiteta tähän DNF:iä
  int paikka = 0;
  while(l && f > l->f) {
    l = l->seur;
    paikka++;
  }
  return paikka;
}

int hae_silistalta(strlista* l, int i) {
  char tmp[10];
  sprintf(tmp, "%i. ", i);
  int r = 0;
  while(l) {
    if(!strcmp(l->str, tmp))
      return r;
    r++;
    l = l->seur;
  }
  return -1;
}

/*i on tuloksen indeksi nollasta alkaen järjestämättä*/
int poista_jarjlistalta(int i) {
  int paikka = hae_silistalta(_yalkuun(sijarj), i+1);
  if(paikka < 0)
    return paikka;; //kyseistä ei ollut jarjlistalla ensinkään
  char palsuunta = (paikka == 0)? 1 : -1;
  _strpoista1(_ynouda(_yalkuun(sijarj), paikka), palsuunta);
  _strpoista1(_ynouda(_yalkuun(strjarj), paikka), palsuunta);
  _yrm1(_ynouda(fjarj, paikka), palsuunta);
  return paikka;
}

/*alku on pienin luku, jota muutetaan eli alku ≥ 1*/
void numerointi_miinus_miinus(strlista* l, int alku) {
  if(!l)
    return;
  char tmpc[15];
  int li;
  do {
    sscanf(l->str, "%i. ", &li);
    if(li < alku)
      continue;
    sprintf(tmpc, "%i. ", li-1);
    l = _strpoista1(l, -1);
    l = _strlisaa_kopioiden(l, tmpc);
  } while((l = l->seur));
}

void lisaa_listoille(char* kello, time_t hetki) {
  char tmp[10];
  strtulos = _strlisaa_kopioiden(strtulos, kello);
  ftulos = _flisaa(ftulos, lue_kellosta(kello));
  tuloshetki = _ilisaa(tuloshetki, hetki);
  int paikka = hae_paikka(ftulos->f, fjarj) - 1; //0. elementti on kansilehti
  flista* tmpfl = fjarj;
  if( (tmpfl = _ynouda(fjarj, paikka)) ) { //noutaminen epäonnistuu, jos f on inf
    _flisaa(tmpfl, ftulos->f);
    _strlisaa_kopioiden(_ynouda(_yalkuun(strjarj), paikka), strtulos->str);
    sprintf(tmp, "%i. ", _ylaske(_yalkuun(ftulos)));
    _strlisaa_kopioiden(_ynouda(_yalkuun(sijarj), paikka), tmp);
  }
}

void poista_listoilta_viimeinen() {
  if(!sektus->pit)
    return;
  int id = sektus->pit-1;
  free(sektus->taul[id]);
  free(stulos->taul[id]);
  sektus->pit--;
  stulos->pit--;
  ftulos->pit--;
  thetki->pit--;
  jarjes->pit--;
}

void poista_listoilta(int ind) { //turha
  if(ind+1 == _ylaske(_yalkuun(strtulos))) {
    strtulos = _strpoista1(strtulos, -1);
    ftulos = _yrm1(ftulos, -1);
    tuloshetki = _yrm1(tuloshetki, -1);
  } else {
    _strpoista1(_ynouda(_yalkuun(strtulos), ind), 1);
    _yrm1(_ynouda(_yalkuun(ftulos), ind), 1);
    _yrm1(_ynouda(_yalkuun(tuloshetki), ind), 1);
  }
  poista_jarjlistalta(ind);
  numerointi_miinus_miinus(_yalkuun(sijarj), ind+1);
}

float lue_kellosta(char* s) {
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

char tallenna(char* tiednimi) {
  flista* ft = _yalkuun(ftulos);
  ilista* th = _yalkuun(tuloshetki);
  if(!ft)
    return 0;
  
  FILE *f = fopen(tiednimi, "r+");
  /*jos tiedostoa ei ollut, se tehdään, muuten haetaan oikea kohta*/
  if(!f) {
    f = fopen(tiednimi, "w");
    goto KIRJOITA;
  }
  
  /*kirjoitetaan jo olleen ajan päälle, jos on samalta hetkeltä tiedostossa*/
  time_t hetki = th->i;
  time_t yrite = 0;
  char c = 0;
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
	fprintf(stderr, "Virhe: Lukeminen epäonnistui, c = %hhx (hexa) '%c'\n", c, c);
	return 0;
      }
    } else {
      if (yrite >= hetki) {
	fseek(f, sij, SEEK_SET); //takaisin rivin alkuun, tämän päälle kirjoitetaan
	goto KIRJOITA;
      }
    }
  }
 KIRJOITA:
  while(ft) {
    fprintf(f, "%.2f\t%i\n", ft->f, th->i);
    ft = ft->seur;
    th = th->seur;
  }
  fclose(f);
  return 1;
}

char lue_tiedosto(char* tiednimi, char* rajaus) {
  FILE *f = fopen(tiednimi, "r");
  if(!f) {
    fprintf(stderr, "Virhe, ei tiedostoa \"%s\"\n", tiednimi);
    return 1;
  }

  float faika;
  int i;
  char c;
  char* kello = kellool.teksti;
  
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
    ftulos = _flisaa(ftulos, faika);
    tuloshetki = _ilisaa(tuloshetki, i);
    strtulos = _strlisaa_kopioiden(strtulos, float_kelloksi(kello, faika));
  }
 LUETTU:
  /*rajataan*/
  if(strlen(rajaus)) {
    int alku, loppu, kpl; //loppu on 1., jota ei tule
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
	loppu = -1;
	break;
      }
    }
    if(loppu < alku)
      goto TEE_LISTA;
    int *poist;
    int pit = _ylaske(_yalkuun(ftulos));
    if(alku < 0)
      alku += pit;
    if(loppu < 0)
      loppu += pit+1;
    kpl = pit-(loppu-alku);
    poist = malloc(kpl*sizeof(int));
    int ind=0;
    for(int i=0; i<alku; i++)
      poist[ind++] = i;
    for(int i=loppu; i<pit; i++)
      poist[ind++] = i;
    if(ind != kpl)
      fprintf(stderr, "Virhe poistossa: %i ≠ %i\n", kpl, ind);
    if(poist) {
      ftulos = _yloppuun(_yrm(_yalkuun(ftulos), poist, kpl));
      tuloshetki = _yloppuun(_yrm(_yalkuun(tuloshetki), poist, kpl));
      strtulos = _yloppuun(_strpoista(_yalkuun(strtulos), poist, kpl));
      free(poist);
    }
  }
 TEE_LISTA:
  tee_jarjlista();
  fclose(f);
  return 0;
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

void tee_jarjlista() {
  char str[50];
  _yrma(fjarj->seur);
  _strpoista_kaikki(strjarj->seur);
  _strpoista_kaikki(sijarj->seur);

  _ylsvlms(fjarj, _fkopioi_palauta_alku(_yalkuun(ftulos), -1)); //fjarj kopioituna järjestämättä
  unsigned pit = _ylaske_taakse(ftulos);
  unsigned* jarjtaul = malloc(pit*sizeof(unsigned));
  float_lomitusjarj_jarj_pit(fjarj->seur, jarjtaul, pit); //fjarj järjestettynä
  
  flista* fjuoksu=fjarj;
  strlista *strjuoksu=strjarj, *sijuoksu=sijarj;
  for(unsigned i=0; i<pit; i++) {
    fjuoksu=fjuoksu->seur;
    if(fjuoksu->f == INFINITY) {
      _yrma(fjuoksu);
      break;
    }
    strjuoksu = _strlisaa_kopioiden(strjuoksu, float_kelloksi(str, fjuoksu->f)); //strjarj
    sprintf(str, "%u. ", jarjtaul[i]+1);
    sijuoksu = _strlisaa_kopioiden(sijuoksu, str); //sijarj
  }
  free(jarjtaul);
}

/*teksti voi olla kello tai turha*/
void muuta_sakko(char* teksti, int ind) {
  strlista* sl = _ynouda(_yalkuun(strtulos), ind);
  if(!sl)
    return;
  float* fp = &( ((flista*)_ynouda(_yalkuun(ftulos), ind))->f );
  int min=0;
  sakko_e sakko = hae_sakko(sl->str);

  /*tuloslistat*/
  switch(sakko) {
  case ei:
    *fp += 2;
    break;
  case plus:
    *fp -= 2;
    break;
  case dnf:
    if(!strstr(sl->str, ":")) {
      sscanf(sl->str, "Ø(%f)", fp);
    } else {
      float fsek;
      sscanf(sl->str, "Ø(%i:%f)", &min, &fsek);
      *fp = min*60 + fsek;
    }
    break;
  }
  min = (int)(*fp+0.00001) / 60;
  
  if(sakko != dnf) //järjestyslistalta poisto ennen sakon muuttamista
    poista_jarjlistalta(ind);
  
  sakko = (sakko + 1) % 3;
  
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
  
  sl->str = realloc(sl->str, strlen(teksti)+1);
  strcpy(sl->str, teksti);
  
  if(sakko == dnf)
    *fp = INFINITY;

  /*järjestyslistat*/
  int paikka = hae_paikka(*fp, fjarj) - 1; //0. elementti on kansilehti
  flista* ftmp = fjarj;
  if( (ftmp = _ynouda(fjarj, paikka)) ) { //noutaminen epäonnistuu, jos f on inf
    _flisaa(ftmp, *fp);
    _strlisaa_kopioiden(_ynouda(_yalkuun(strjarj), paikka), teksti);
    char tmp[15];
    sprintf(tmp, "%i. ", ind+1);
    _strlisaa_kopioiden(_ynouda(_yalkuun(sijarj), paikka), tmp);
  }
}

sakko_e hae_sakko(char* s) {
  if(strstr(s, "Ø"))
    return dnf;
  if(strstr(s, "+"))
    return plus;
  return ei;
}
