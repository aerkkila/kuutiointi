/*
  Lasketaan monellako siirrolla pääsee takaisin alkuun toistamalla jotain yhdistelmää.
  Tämä liitetään includella pääfunktioon, mutta gcc sallii määritellä funktioita myös funktion sisällä.
 
  1. siirto on aina r[], koska kuutiota kääntämällä muut siirrot voidaan muuntaa siksi.
  2. siirto on aina l[] tai u[], koska kuutiota kääntämällä muut voidaan muuttaa niiksi.
  
  Seuraavia ei ole vielä toteutettu:
  Yleisesti sanotaan esimerkiksi, että 1. siirto on x-akselin ympäri positiiviseen suuntaan positiiviselta puolelta
  ja 2. on y-akselin ympäri positiiviseen suuntaan positiiviselta puolelta.
  1. siirto määrittelee x-akselin, mutta ei suuntaa, jos tehdään perään toiselta puolelta eri suuntaan, esim R L.
  1. siirto määrittelee myös suunnan, jos tehdään R L'. Molemmat ovat samaan suuntaan, joten myötäpäivä saadaan määriteltyä.
  1. siirto määrittelee suunnan silloinkin, jos se on R2, jota ei seuraa L2.
  Tämä kiinnittää x-akselin positiivisen puolen ja siten myös suunnan.
  2. siirto määrittelee väkisin y-akselin (R L yms. laskettiin yhdeksi siirroksi)
  Esim. R2 L2 U' -sarjan jälkeen y2 on mahdollinen, koska R2 L2 kiinnittää x-akselin suunnatta ja U' kiinnittää y-akselin suuntineen
  R2 L2 U' D -sarjan jälkeen myös x2 tai z2 on mahdollinen, koska kaikki suunnat ovat kiinnittymättä
 */

static const char* tahkot = "RUFLDB";
static const char* suunnat = " '2";
static const int isuunnat[] = {1,3,2};

#define MAXPIT 5
#define PIT_SARJA (MAXPIT*3+1)
#define PIT_ISARJA (MAXPIT*2*sizeof(int))
char* sarja = malloc(PIT_SARJA);
int* isarja = malloc(PIT_ISARJA);
char* sarja0 = malloc(PIT_SARJA);
int* isarja0 = malloc(PIT_ISARJA);
memset(sarja,suunnat[0],PIT_SARJA);
for(int i=0; i<MAXPIT*2; i++)
  isarja[i] = isuunnat[0];
sarja[PIT_SARJA-1] = '\0';

long powi(long a, int b) {
  long r = 1;
  for(; b; b--)
    r *= a;
  return r;
}

/*kun 'luku' muutetaan 'kanta'-kantaiseksi, mikä on vähiten merkitsevästä alkaen i. numero*/
#define NLUKU(luku, i, kanta) ( (luku) % powi(kanta,(i)+1) / powi(kanta,i) )

/*merkitään siirtotahkot kuusikantaisilla 'sexaluvuilla'
  esim. 0135 olisi R U L B
  lisättäessä 1 saadaan 0140 eli R U D R*/

/*Ei tiedetä, mikä on oikea järjestys kutsua tahkon ja akselin korjausfunktioita.
  Käydään rekursiolla läpi kaikki polut ja valitaan se, joka tuottaa pienimmän luvun.
  Kumpikin funktio kutsuu siksi kumpaakin funktiota aina, kun jotain muutetaan
  korjaa_sexa() huolehtii tästä, sitä kutsutaan molemmissa funktioissa.*/
auto long korjaa_sexa_tahko(long, int); //luokka pitää olla auto, kun funktio kuulutetaan funktion sisällä
auto long korjaa_sexa_akseli(long, int);

long korjaa_sexa(long sexa, int pit) {
  long tah = korjaa_sexa_tahko(sexa, pit);
  long aks = korjaa_sexa_akseli(sexa, pit);
  if(tah==sexa) return aks;
  if(aks==sexa) return tah;
  return (tah<aks)? tah: aks;
}

/*sama tahko ei saa esiintyä kahdesti peräkkäin*/
long korjaa_sexa_tahko(long sexa, int pit) {
  if(pit < 2)
    return sexa;
  for(int i=pit-1; i>=1; i--) //luku käydään eniten merkitsevästä vähiten merkitsevään
    if (NLUKU(sexa, i, 6) == NLUKU(sexa, i-1, 6)) { //jos viereiset siirrot ovat samoja
      sexa += powi(6,i-1); //vähemmän merkitsevää (oikeanpuoleista) kasvatetaan yhdellä
      return korjaa_sexa(sexa, pit);
    }
  return sexa;
}

/*Sama siirtoakseli ei saa esiintyä kolmesti peräkkäin, esim R L R.
  Samalla akselilla mod(tahkon_luku,3) on vakio.*/
long korjaa_sexa_akseli(long sexa, int pit) {
  if(pit < 3)
    return sexa;
  int a = NLUKU(sexa, pit-1, 6);
  int b = NLUKU(sexa, pit-2, 6);
  int c = NLUKU(sexa, pit-3, 6);
  for(int i=pit-1;;) {
    if (a%3 == b%3 && a%3 == c%3) {
      sexa += powi(6,i-2);
      return korjaa_sexa(sexa, pit);
    }
    if(--i < 2)
      return sexa;
    a=b; b=c;
    c = NLUKU(sexa, i-2, 6);
  }
}

int sarjantekofunktio(int pit, char* sarja, int* isarja) {
  static long sexa = 0;
  static long trexa = -1;
  if(!sexa)
    sexa = korjaa_sexa(sexa,pit);
  if(++trexa >= powi(3,pit)) { //jos kaikki suuntayhdistelmät on käytetty
    trexa = 0;
    sexa = korjaa_sexa(++sexa, pit);
    if( pit > 1) {
      int n2luku = NLUKU(sexa, pit-2, 6);
      if( n2luku > _l ) { //jos 2. siirto on ohittanut L[]:n, kaikki on jo käytetty
	trexa=-1;
	sexa=0;
	return 1;
      } else if ( n2luku == _f ) {
	sexa += powi(6,pit-2);
	sexa = korjaa_sexa(sexa, pit);
      }
    } else {
      if( NLUKU(sexa, pit-1, 6) ) { //jos 1. siirto ei ole enää R[] eli kaikki tahkoyhdistelmät on käytetty
	trexa=-1;
	sexa=0;
	return 1;
      }
    }
  }
  for(int i=0; i<pit; i++) {
    int ind = NLUKU(sexa, i, 6);
    sarja[(pit-i-1)*3] = tahkot[ind];
    isarja[(pit-i-1)*2] = ind;
    ind = NLUKU(trexa, i, 3);
    sarja[(pit-i-1)*3+1] = suunnat[ind];
    isarja[(pit-i-1)*2+1] = isuunnat[ind];
  }
  return 0;
}

for(int pit=1; pit<=MAXPIT; pit++) {
  int sarjoja = 0;
  sarja[pit*3] = '\0';
  while(!sarjantekofunktio(pit,sarja,isarja)) {
    unsigned kohta=0;
    int lasku=0;
    do {
      siirto(isarja[kohta*2], 1, isarja[kohta*2+1]);
      kohta = (kohta+1) % pit;
      lasku++;
    } while(!onkoRatkaistu());
    printf("%s\t%i\t%i\t%i\n", sarja,pit,++sarjoja,lasku);
  }
 }

free(sarja); free(isarja);
free(sarja0); free(isarja0);
sarja = sarja0 = NULL;
isarja = isarja0 = NULL;
#undef MAXPIT
#undef PIT_SARJA
#undef PIT_ISARJA
