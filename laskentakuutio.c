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

auto long korjaa_sexa_tahko(long, int); //Luokka pitää olla auto, kun funktio kuulutetaan funktion sisällä.
auto long korjaa_sexa_akseli(long, int);
auto long korjaa_sexa(long, int);
auto int sarjantekofunktio(int pit, long *sexa, long *trexa);
auto long powi(long a, int b);

auto void* laskenta(void* vp);

/*kun 'luku' muutetaan 'kanta'-kantaiseksi, mikä on vähiten merkitsevästä alkaen i. numero*/
#define NLUKU(luku, i, kanta) ( (luku) % powi(kanta,(i)+1) / powi(kanta,i) )

/*merkitään siirtotahkot kuusikantaisilla "sexaluvuilla"
  esim. 0135 olisi R U L B
  lisättäessä 1 saadaan 0140 eli R U D R*/

/*komentoriviargumentit*/
int maxpit_l = 4;
int toita = 1;
for(int i=0; i<argc-1; i++)
  if(!strcmp(argv[i], "--maxpit")) {
    if(!(sscanf(argv[i+1], "%i", &maxpit_l)))
      puts("Ei luettu maksimipituutta");
  } else if(!strcmp(argv[i], "--töitä")) {
    if(!(sscanf(argv[i+1], "%i", &toita)))
      puts("Ei luettu töitten määrää");
  }
pthread_t saikeet[toita];

typedef struct {
  long sexa;
  long raja;
  int pit;
  int id;
} saikeen_tiedot;

void* ohita(void*) {
  return NULL;
}

/*tämän osion pääfunktio*/
#ifndef DEBUG
for(int pit=1; pit<=maxpit_l; pit++) {
#else
  int pit = maxpit_l;
#endif
  long sexa0 = 0; long trexa=-1;
  if(sarjantekofunktio(pit, &sexa0, &trexa))
#ifndef DEBUG
    continue;
#else
    return 0;
#endif
  long sexa1 = powi(6,pit-1); //rajana on, että ensimmäinen siirto on U[] eli 1
  long patka = (sexa1-sexa0)/toita;
  saikeen_tiedot t[toita];
  for(int i=0; i<toita; i++) {
    long raja0 = sexa0+patka*i;
    long raja1 = sexa0+patka*(i+1);
    if(raja1==raja0) {
      pthread_create(saikeet+i, NULL, ohita, t+i);
      continue;
    }
    if(i==toita-1)
      raja1 = sexa1;
    t[i] = (saikeen_tiedot){.sexa=raja0, .raja=raja1, .pit=pit, .id=i};
    pthread_create(saikeet+i, NULL, laskenta, t+i);
  }
  for(int i=0; i<toita; i++)
    pthread_join(saikeet[i], NULL);
  /*Alustetaan uusi tiedosto*/
  char apu[22];
  char snimi[12];
  sprintf(apu, "sarjat%i.csv", pit);
  FILE *ulos = fopen(apu, "w");
  fprintf(ulos, "sarja\tsiirtoja\n");
  /*Liitetään siihen säikeitten tekemät tiedostot*/
  char c;
  for(int i=0; i<toita; i++) {
    sprintf(snimi, "tmp%i.csv", i);
    FILE *f = fopen(snimi, "r");
    if(!f)
      continue;
    while((c=fgetc(f)) > 0)
      fputc(c, ulos);
    fclose(f);
#ifndef DEBUG
    sprintf(apu, "rm %s", snimi);
    system(apu);
#endif
  }
  fclose(ulos);
#ifndef DEBUG
 }
#endif

#define PIT_SARJA (pit*3)
#define PIT_ISARJA (pit*2*sizeof(int))
void* laskenta(void* vp) {
  kuutio_t kuutio1 = kuutio; //säikeet eivät saa käyttää yhteistä kuutiota
  kuutio1.sivut = malloc(6*kuutio1.N2);
  memcpy(kuutio1.sivut, kuutio.sivut, 6*kuutio1.N2);
  saikeen_tiedot tied = *(saikeen_tiedot*)vp;
  int pit = tied.pit;
  long trexa = -1;
  tied.sexa = korjaa_sexa(tied.sexa, pit); //korjataan alkutila ja myöhemmin pidetään oikeana
  char* sarja = malloc(PIT_SARJA+1);
  int* isarja = malloc(PIT_ISARJA);
  char* sarja0 = malloc(PIT_SARJA+1);
  int* isarja0 = malloc(PIT_ISARJA);
  memset(sarja,suunnat[0],PIT_SARJA+1);
  for(int i=0; i<pit*2; i++)
    isarja[i] = isuunnat[0];
  sarja[PIT_SARJA] = '\0';

  char nimi[10];
  sprintf(nimi, "tmp%i.csv", tied.id);
  FILE *f = fopen(nimi, "w");
  do {
    if ( !(tied.sexa < tied.raja) )
      break;
    unsigned kohta=0;
    int lasku=0;
    /*sarja kirjalliseen ja lukumuotoon*/
    for(int i=0; i<pit; i++) {
      int ind = NLUKU(tied.sexa, i, 6);
      sarja[(pit-i-1)*3] = tahkot[ind];
      isarja[(pit-i-1)*2] = ind;
      ind = NLUKU(trexa, i, 3);
      sarja[(pit-i-1)*3+1] = suunnat[ind];
      isarja[(pit-i-1)*2+1] = isuunnat[ind];
    }
    do {
      siirto(&kuutio1, isarja[kohta*2], 1, isarja[kohta*2+1]);
      kohta = (kohta+1) % pit;
      lasku++;
    } while(!onkoRatkaistu(&kuutio1));
    fprintf(f, "%s\t%i\n", sarja,lasku);
  } while(!sarjantekofunktio(pit,&tied.sexa,&trexa));
  fclose(f);
  free(sarja); free(isarja);
  free(sarja0); free(isarja0);
  free(kuutio1.sivut);
  return NULL;
}
#undef PIT_SARJA
#undef PIT_ISARJA

long powi(long a, int b) {
  long r = 1;
  for(; b; b--)
    r *= a;
  return r;
}

/*Ei tiedetä, mikä on oikea järjestys kutsua tahkon ja akselin korjausfunktioita.
  Käydään rekursiolla läpi kaikki polut ja valitaan se, joka tuottaa pienimmän luvun.
  Kumpikin funktio kutsuu siksi kumpaakin funktiota aina, kun jotain muutetaan
  korjaa_sexa() huolehtii tästä, sitä kutsutaan molemmissa funktioissa.*/
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

inline int onkoLoppu(long *sexa, int pit) {
  if( pit <= 1 )
    return *sexa;
  int n2luku = NLUKU(*sexa, pit-2, 6);
  if( n2luku > _l ) //jos 2. siirto on ohittanut L[]:n, kaikki on jo käytetty
    return 1;
  else if( n2luku == _f) {
    *sexa += powi(6,pit-2);
    *sexa = korjaa_sexa(*sexa, pit);
  }
  return 0;
}

int sarjantekofunktio(int pit, long *sexa, long *trexa) {
  if(onkoLoppu(sexa, pit))
    return 1;
#ifndef DEBUG //suuntia ei käsitellä virheenjäljityksessä
  if(++trexa[0] >= powi(3,pit)) { //jos kaikki suuntayhdistelmät on käytetty
#endif
    *trexa = 0;
    *sexa = korjaa_sexa(++sexa[0], pit);
    return onkoLoppu(sexa, pit);
#ifndef DEBUG
  }
#endif
  return 0;
}
