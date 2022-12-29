#define kynnyskerroin 4.5
#define KYNNYS_SIETO 0.001

#define PERUS 0
#define SUODATE 1
#define DERIV 2
#define OHENNE 3

typedef struct {int a[2];} int2;

#ifdef ABS
#undef ABS
#endif
#define ABS(a) ((a)<0? -(a): a)

extern lpuu_matriisi* xmat;
extern char* yarr;
extern lpuu* puu;
extern int2* luokitus_sij;
extern int luokituksia;

/* Minkä arvon ylittäneet arvot lasketaan piikeiksi.
   Määritetään laskemalla keskiarvo ja keskihajonta.
   Hylkäysrajojen ulkopuoliset luvut hylätään laskettaessa keskiarvoa,
   mikä mahdollistaa kynnysarvon määrittämisen iteratiivisesti.
   */
static void laske_kynnysarvo(const float* data, int pit0, float hylkraja0, float hylkraja1, float* kynnarv) {
    double avg = 0;
    int pit = 0;
    for(int i=0; i<pit0; i++) {
	if(data[i]<=hylkraja0 || hylkraja1<=data[i] || !(data[i]==data[i])) // näin päin hylkraja saa olla epäluku
	    continue;
	avg += data[i];
	pit++;
    }
    avg /= pit;
    double std = 0;
    for(int i=0; i<pit0; i++)
	if(!(data[i]<=hylkraja0 || hylkraja1<=data[i] || !(data[i]==data[i])))
	    std += (data[i]-avg)*(data[i]-avg);
    std = sqrt(std/pit);
    kynnarv[1] = avg+std*kynnyskerroin;
    kynnarv[0] = avg-std*kynnyskerroin;
}

static void iteroi_kynnysarvo(const float* data, int pit, float* kynnarv, float sieto) {
    float vanhat[2] = {kynnarv[0], kynnarv[1]};
    for(int i=0; i<200; i++) {
	laske_kynnysarvo(data, pit, kynnarv[0], kynnarv[1], kynnarv);
	if(ABS(vanhat[0]-kynnarv[0]) < sieto && ABS(vanhat[1]-kynnarv[1]) < sieto)
	    return;
	vanhat[0] = kynnarv[0];
	vanhat[1] = kynnarv[1];
    }
    printf("\033[93mVaroitus:\033[0m iterointi saavutti maksimipituuden supistumatta (%s: %i).\n", __FILE__, __LINE__);
}

static float _maksimi(const float* data, int alku, int loppu) {
    float max = -INFINITY;
    for(int i=alku; i<loppu; i++)
	if(data[i] > max)
	    max = data[i];
    return max;
}

static float _kynnysarvon_ylitys(const float* data, int raidan_pit, int alku, int loppu) {
    float kynnarv[2];
    kynnarv[0] = kynnarv[1] = NAN;
    iteroi_kynnysarvo(data, raidan_pit, kynnarv, KYNNYS_SIETO);
    return _maksimi(data, alku, loppu) / kynnarv[1];
}

/* Tämä alkeellinen versio vain päällekirjoittaa mahdollisen vanhan datan.
   globaalit ulos:   xmat;
   globaalit sisään: luokituksia, luokitus_sij;
   */
static void tee_opetusdata(const float* data, int raidan_pit) {
    int pit1 = luokituksia, pit2 = 1 + 2;
    if(xmat)
	xmat->data = realloc(xmat->data, pit1*pit2*sizeof(float));
    else {
	xmat = malloc(sizeof(lpuu_matriisi));
	xmat->data = malloc(pit1*pit2*sizeof(float));
    }
    xmat->pit1 = pit1;
    xmat->pit2 = pit2;
    int ind = 0;
    for(int i=0; i<luokituksia; i++) {
	// kesto
	xmat->data[ind++] = luokitus_sij[i].a[1] - luokitus_sij[i].a[0];
	// kynnysarvon ylitys [molemmat]
	xmat->data[ind++] = _kynnysarvon_ylitys(data+SUODATE*raidan_pit, raidan_pit, luokitus_sij[i].a[0], luokitus_sij[i].a[1]);
	xmat->data[ind++] = _kynnysarvon_ylitys(data+DERIV*raidan_pit,   raidan_pit, luokitus_sij[i].a[0], luokitus_sij[i].a[1]);
	// huipun sijainti ajassa / kesto [molemmat]
	//xmat->data[ind++] = argmax(SUODATE, luokitus_sij[i].a[0], luokitus_sij[i].a[1]) / xmat->data[i-1];
	//xmat->data[ind++] = argmax(DERIV,   luokitus_sij[i].a[0], luokitus_sij[i].a[1]) / xmat->data[i-1];
	// ohenteen pisteet
	//xmat->data[ind++] = (float)laske_ohenne(luokitus_sij[i].a[0], luokitus_sij[i].a[1]);
    }
    assert(ind == xmat->pit1*xmat->pit2);
}
