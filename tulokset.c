#include <lista_math.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tulokset.h"
#include "rakenteet.h"

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
  if(_ylaske(l) != n)
    return NAN;
  flista* alku = l;
  floatjarjesta(l, floatmin, NULL, -1);
  for(int i=0; i<pois && l; i++)
    l = l->seur;
  double r = floatstd(l, n-(pois*2));
  _yrma(alku);
  return r;
}

strlista* tee_tiedot(strlista* strtiedot, flista* fl, int* avgind) {
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
  sprintf(tmp, " = %.2lf (σ = %.2f)",		\
	  floatmean(_yalkuun(fl), -1), floatstd(_yalkuun(fl), -1));
  strtiedot = _strlisaa_kopioiden(strtiedot, tmp);

  /*Mediaani*/
  floatint med = floatmed(_yalkuun(fl), -1);
  sprintf(tmp, " = %.2f (%i.)", med.a, med.b+1);
  strtiedot = _strlisaa_kopioiden(strtiedot, tmp);
  free(tmp);
  return strtiedot;
}

/*laittaa yhden suurimman ja pienimmän sulkeisiin*/
strlista* tee_lisatiedot(tkset_s* t, strlista* sektus, int alkuind, int n) {
  strlista* r = NULL;
  char tmp[150];
  char aikastr[100];
  strlista* sl = _ynouda(_yalkuun(t->strtulos), alkuind);
  flista* fl = _ynouda(_yalkuun(t->ftulos), alkuind);
  if(sektus) sektus = _ynouda(_yalkuun(sektus), alkuind);
  if(!sl)
    return NULL;

  /*tehdään aikastring*/
  time_t aika_t = ((ilista*)_ynouda(_yalkuun(t->tuloshetki), alkuind+n-1))->i;
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
void poista_jarjlistalta(int i, strlista** si, strlista** s, flista** fl) {
  int paikka = hae_silistalta(_yalkuun(*si), i+1);
  if(paikka < 0)
    goto NUMEROINTI; //kyseistä ei ollut jarjlistalla ensinkään
  char palsuunta = (paikka == 0)? 1 : -1;
  *si = _strpoista1(_ynouda(_yalkuun(*si), paikka), palsuunta);
  *s  = _strpoista1(_ynouda(_yalkuun(*s ), paikka), palsuunta);
  *fl = _yrm1(_ynouda(_yalkuun(*fl), paikka), palsuunta);

 NUMEROINTI:;
  /*korjataan numerointi, jos poistettiin välistä*/
  /*haetaan ensin maksimi si-listalta*/
  strlista *l = _yalkuun(*si);
  int maks = 0;
  int yrite = 0;
  while(l) {
    sscanf(l->str, "%i", &yrite);
    if(yrite > maks)
      maks = yrite;
    l = l->seur;
  }

  /*sitten korjataan*/
  l = _yalkuun(*si);
  strlista* apu;
  char tmpc[10];
  for(int luku = i+1; luku<=maks; luku++) {
    paikka = hae_silistalta(l, luku);
    if(paikka<0)
      continue;
    sprintf(tmpc, "%i. ", luku-1);
    apu = _ynouda(l, paikka);
    if(*si == apu)
      *si = apu->edel;
    apu = _strpoista1(apu, -1);
    _strlisaa_kopioiden(apu, tmpc);
  }
}

void lisaa_listoille(tkset_s* t, char* kello, time_t hetki, int* aikoja) {
  char tmp[10];
  t->strtulos = _strlisaa_kopioiden(t->strtulos, kello);
  t->ftulos = _flisaa(t->ftulos, lue_kellosta(kello));
  t->tuloshetki = _ilisaa(t->tuloshetki, hetki);
  int paikka = hae_paikka(t->ftulos->f, _yalkuun(t->fjarj)) - 1; //0. elementti on kansilehti
  if( (t->fjarj = _ynouda(_yalkuun(t->fjarj), paikka)) ) { //noutaminen epäonnistuu, jos f on inf
    _flisaa(t->fjarj, t->ftulos->f);
    _strlisaa_kopioiden(_ynouda(_yalkuun(t->strjarj), paikka), t->strtulos->str);
    sprintf(tmp, "%i. ", ++(*aikoja));
    _strlisaa_kopioiden(_ynouda(_yalkuun(t->sijarj), paikka), tmp);
  }
}

void poista_listoilta(tkset_s* t, int ind) {
  if(ind+1 == _ylaske(_yalkuun(t->strtulos))) {
    t->strtulos = _strpoista1(t->strtulos, -1);
    t->ftulos = _yrm1(t->ftulos, -1);
    t->tuloshetki = _yrm1(t->tuloshetki, -1);
  } else {
    _strpoista1(_ynouda(_yalkuun(t->strtulos), ind), 1);
    _yrm1(_ynouda(_yalkuun(t->ftulos), ind), 1);
    _yrm1(_ynouda(_yalkuun(t->tuloshetki), ind), 1);
  }
  poista_jarjlistalta(ind, &(t->sijarj), &(t->strjarj), &(t->fjarj));
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

#if 0
struct tm aikaero(time_t t1, time_t t2) {
  time_t ero = abs(t2-t1);
  struct tm aika = localtime(&ero);
  return aika;
}
#endif

char tallenna(tkset_s* t, char* tiednimi) {
  flista* ft = _yalkuun(t->ftulos);
  ilista* th = _yalkuun(t->tuloshetki);
  if(!ft)
    return 1;
  
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
  while(ft) {
    fprintf(f, "%.2f\t%i\n", ft->f, th->i);
    ft = ft->seur;
    th = th->seur;
  }
  fclose(f);
  return 0;
}
