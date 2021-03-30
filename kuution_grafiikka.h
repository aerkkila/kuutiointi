#include "kuutio.h"

#ifndef __KUUTION_GRAFIIKKA__
#define __KUUTION_GRAFIIKKA__
#define RUUTU(tahko, i, j) (((tahko)*kuutio.N*kuutio.N + (i)*kuutio.N + (j))*4)
#endif

void tee_ruutujen_koordtit();
koordf ruudun_nurkka(int tahko, int iRuutu, int jRuutu, int nurkkaInd);
void piirra_suunnikas(void* koordf2tai3, int onko2vai3);
koordf2* jarjestaKoord2(koordf2* ret, koordf2* ktit, int akseli, int pit);
koordf* jarjestaKoord(koordf* ret, koordf* ktit, int akseli, int pit);
void piirra_kuvaksi(int tahko);
void piirra_viiva(void* karg1, void* karg2, int onko2vai3, int paksuus);
void korosta_tahko(int tahko);
void kaantoanimaatio(int tahko, double maara, double aika);
