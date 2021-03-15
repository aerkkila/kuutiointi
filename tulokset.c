#include <lista_math.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cfg.h"
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

strlista* tee_tiedot(strlista* strtiedot, tkset_s* tkset, int* avgind) {
  flista* fl = tkset->ftulos;
  flista* fj = tkset->fjarj;
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
  /*karsitaan niin monta kuin on DNF:iä jättämällä erotus pois järjestyslistan alusta*/
  int ero = _ylaske_taakse(fl) - (_ylaske(fj)-1);
  sprintf(tmp, " = %.2lf (σ = %.2f)",		\
	  floatmean(_ynouda(fj, ero+1), -1), floatstd(_ynouda(fj, ero+1), -1));
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
int poista_jarjlistalta(int i, tkset_s* t) {
  int paikka = hae_silistalta(_yalkuun(t->sijarj), i+1);
  if(paikka < 0)
    return paikka;; //kyseistä ei ollut jarjlistalla ensinkään
  char palsuunta = (paikka == 0)? 1 : -1;
  _strpoista1(_ynouda(_yalkuun(t->sijarj), paikka), palsuunta);
  _strpoista1(_ynouda(_yalkuun(t->strjarj), paikka), palsuunta);
  _yrm1(_ynouda(t->fjarj, paikka), palsuunta);
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

void lisaa_listoille(tkset_s* t, char* kello, time_t hetki) {
  char tmp[10];
  t->strtulos = _strlisaa_kopioiden(t->strtulos, kello);
  t->ftulos = _flisaa(t->ftulos, lue_kellosta(kello));
  t->tuloshetki = _ilisaa(t->tuloshetki, hetki);
  int paikka = hae_paikka(t->ftulos->f, t->fjarj) - 1; //0. elementti on kansilehti
  flista* tmpfl = t->fjarj;
  if( (tmpfl = _ynouda(t->fjarj, paikka)) ) { //noutaminen epäonnistuu, jos f on inf
    _flisaa(tmpfl, t->ftulos->f);
    _strlisaa_kopioiden(_ynouda(_yalkuun(t->strjarj), paikka), t->strtulos->str);
    sprintf(tmp, "%i. ", _ylaske(_yalkuun(t->ftulos)));
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
  poista_jarjlistalta(ind, t);
  numerointi_miinus_miinus(_yalkuun(t->sijarj), ind+1);
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

char tallenna(tkset_s* t, char* tiednimi) {
  flista* ft = _yalkuun(t->ftulos);
  ilista* th = _yalkuun(t->tuloshetki);
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

char lue_tiedosto(char* tiednimi) {
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
    tkset.ftulos = _flisaa(tkset.ftulos, faika);
    tkset.tuloshetki = _ilisaa(tkset.tuloshetki, i);
    tkset.strtulos = _strlisaa_kopioiden(tkset.strtulos, float_kelloksi(kello, faika));
  }
 LUETTU:
  tee_jarjlista(&tkset);
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

void tee_jarjlista(tkset_s* t) {
  char str[50];
  flista* ft = _yalkuun(t->ftulos);
  flista* fj = t->fjarj;
  strlista* sj = t->strjarj;
  strlista* sij = t->sijarj;
  flista* apuf;
  strlista* apus;
  int i=1;
  while(ft) {
    int jarji = hae_paikka(ft->f, fj)-1;
    apuf = _ynouda(fj, jarji); //fjarj
    if(!apuf) {
      ft = ft->seur; //ei löydy, jos on inf
      continue;
    }
    _flisaa(apuf, ft->f);
    apus = _ynouda(sj, jarji); //strjarj
    _strlisaa_kopioiden(apus, float_kelloksi(str, ft->f));
    apus = _ynouda(sij, jarji); //sijarj
    sprintf(str, "%i. ", i++);
    _strlisaa_kopioiden(apus, str);
    ft = ft->seur;
  }
}


/*teksti voi olla kello tai turha*/
void muuta_sakko(tkset_s* t, char* teksti, int ind) {
  strlista* sl = _ynouda(_yalkuun(t->strtulos), ind);
  if(!sl)
    return;
  float* fp = &( ((flista*)_ynouda(_yalkuun(t->ftulos), ind))->f );
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
    poista_jarjlistalta(ind, t);
  
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
  int paikka = hae_paikka(*fp, t->fjarj) - 1; //0. elementti on kansilehti
  flista* ftmp = t->fjarj;
  if( (ftmp = _ynouda(t->fjarj, paikka)) ) { //noutaminen epäonnistuu, jos f on inf
    _flisaa(ftmp, *fp);
    _strlisaa_kopioiden(_ynouda(_yalkuun(t->strjarj), paikka), teksti);
    char tmp[15];
    sprintf(tmp, "%i. ", ind+1);
    _strlisaa_kopioiden(_ynouda(_yalkuun(t->sijarj), paikka), tmp);
  }
}

sakko_e hae_sakko(char* s) {
  if(strstr(s, "Ø"))
    return dnf;
  if(strstr(s, "+"))
    return plus;
  return ei;
}
