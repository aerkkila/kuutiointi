#ifndef __kuutio__
#define __kuutio__
#include <SDL2/SDL.h>

/* vain sivut ja indeksit[0] ovat vapautettavia */
typedef struct {
    char* sivut;
    char* apu;
    int *indeksit[3];
    int N; //NxNxN-kuutio
    int N2; //N²
    char ratkaistu;
} kuutio_t;

typedef struct{ unsigned char v[3]; } vari;
typedef struct{ float a[3]; } koordf;
typedef struct {float a[2];} koordf2;
typedef struct {int a[3];} int3;

#define SIGN(i) ((i<0)? -1: 1)
#define ABS(i) ((i<0)? -i: i)
#define SIVU(N, tahko, i, j) ((tahko)*(N)*(N) + (j)*(N) + (i))
#define SIVU2(N2, N, tahko, j, i) ((tahko)*(N2) + (j)*(N) + (i))
#define SIVUINT3(N,A) SIVU(N, (A).a[0], (A).a[1], (A).a[2])
#define VAIHDA(a,b,tyyppi) do {			\
	tyyppi apu_makro_VAIHDA = a;		\
	a = b;					\
	b = apu_makro_VAIHDA;			\
    } while(0)
#define VAIHDA_XOR(a,b) do {a^=b; b^=a; a^=b;} while(0)

#define _r 0
#define _u 1
#define _f 2
#define _l 3
#define _d 4
#define _b 5
const char* tahkojärjestys = "RUFLDB";

extern kuutio_t kuutio;

void luo_kuutio(kuutio_t*, int);
int onkoRatkaistu(kuutio_t*);
void siirto(kuutio_t* kuutio, int tahko, int kaista, int maara);
void paivita();
int3 hae_ruutu(int kuutio_N, int tahko, int i0, int j0);
int3 hae_siivu(int3 ruutu);

#endif
