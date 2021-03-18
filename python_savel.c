#include <stdlib.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "python_savel.h"

float* savelmuistiin() {
  int avain = ftok("/tmp", 44);
  if(avain < 0) {
    perror("Virhe, ftok");
    return NULL;
  }
  int shmid = shmget(avain, sizeof(float), IPC_CREAT | 0666);
  if(shmid < 0) {
    perror("Virhe, shmget");
    return NULL;
  }
  float *r = shmat(shmid, NULL, 0);
  if(r < 0) {
    perror("Virhe, shmat");
    return NULL;
  }
  return r;
}

int savel_ero(float savel) {
  static float alkusavel = -1.0;
  if(alkusavel < 0) {
    alkusavel = savel;
    printf("Alkusävel: %f.0\n", alkusavel);
    return 0x7fffffff;
  }
  return round(12 * log(savel/alkusavel) / log(2));
}

void* sulje_savelmuisti(void *ptr) {
  if(system("pkill sävel.py") < 0)
    fprintf(stderr, "Virhe ohjelman \"sävel.py\" sulkemisessa\n");
  if(shmdt(ptr) < 0 )
    perror("shmdt");
  return NULL;
}
