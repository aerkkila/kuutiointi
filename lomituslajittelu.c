#include <stdlib.h>

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
